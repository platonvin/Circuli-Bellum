.ONESHELL:

#setting up include and lib directories for dependencies
I = -Isrc -Icommon -Ilum-al/src -Ibox2d/include -Itw-colors-cpp
L = -Llum-al/lib -Lbox2d/build/src/

CPP_COMPILER = g++
# CPP_COMPILER = clang++

STATIC_OR_DYNAMIC = 
REQUIRED_LIBS = -llumal -lglfw3 -lvolk -lbox2d
ifeq ($(OS),Windows_NT)
	REQUIRED_LIBS += -lgdi32 -lwinmm
	STATIC_OR_DYNAMIC += -static
else
	REQUIRED_LIBS += -lpthread -ldl
endif
	
always_enabled_flags = -fno-exceptions -std=c++20 -flto
debug_flags   = -O0 -g 
# TODO -fsanitize=undefined
release_flags = -Os -mmmx -msse4 -mpclmul -s -flto

crazy_flags   = -flto -fopenmp -floop-parallelize-all -ftree-parallelize-loops=8 -D_GLIBCXX_PARALLEL -funroll-loops -w $(release_flags)

SHADER_FLAGS = --target-env=vulkan1.1 -g -O

srcs := \
	src/main.cpp\
	src/physics.cpp\
	src/visual.cpp\
	src/logic.cpp\
	src/input.cpp\
	src/actors/player.cpp\
	src/actors/projectile.cpp\
	src/actors/scenery.cpp\

deb_objs := $(patsubst src/%.cpp, obj/deb/%.o, $(srcs))
rel_objs := $(patsubst src/%.cpp, obj/rel/%.o, $(srcs))

#default target
all: setup release

#rule for re-evaluation after vcpkg_installed created
.PHONY: vcpkg_installed_eval 
vcpkg_installed_eval: vcpkg_installed
	$(eval OTHER_DIRS := $(filter-out vcpkg_installed/vcpkg, $(wildcard vcpkg_installed/*)) )
	$(eval INCLUDE_LIST := $(addsuffix /include, $(OTHER_DIRS)) )
	$(eval INCLUDE_LIST += $(addsuffix /include/vma, $(OTHER_DIRS)) )
	$(eval INCLUDE_LIST += $(addsuffix /include/volk, $(OTHER_DIRS)) )
	$(eval LIB_LIST := $(addsuffix /lib, $(OTHER_DIRS)) )
	$(eval I += $(addprefix -I, $(INCLUDE_LIST)) )
	$(eval L += $(addprefix -L, $(LIB_LIST)) )
	$(eval GLSLC_DIR := $(firstword $(foreach dir, $(OTHER_DIRS), $(wildcard $(dir)/tools/shaderc))) )
	$(eval GLSLC := $(strip $(GLSLC_DIR))/glslc )

setup: init vcpkg_installed_eval lum-al/lib/liblumal.a
#If someone knows a way to simplify this, please tell me 
obj/rel/%.o: setup
obj/rel/%.o: src/%.cpp
	$(CPP_COMPILER) $(release_flags) $(always_enabled_flags) $(I) $(args) -MMD -MP -c $< -o $@
DEPS = $(rel_objs:.o=.d)
-include $(DEPS)

obj/deb/%.o: setup
obj/deb/%.o: src/%.cpp
	$(CPP_COMPILER) $(debug_flags) $(always_enabled_flags) $(I) $(args) -MMD -MP -c $< -o $@
DEPS = $(deb_objs:.o=.d)
-include $(DEPS)


SHADER_SRC_DIR = shaders
SHADER_OUT_DIR = shaders/compiled

_SHADERS += $(wildcard $(SHADER_SRC_DIR)/*.vert)
_SHADERS += $(wildcard $(SHADER_SRC_DIR)/*.frag)
_TARGETS = $(patsubst $(SHADER_SRC_DIR)/%, $(SHADER_OUT_DIR)/%.spv, $(_SHADERS))

$(SHADER_OUT_DIR)/%.spv: $(SHADER_SRC_DIR)/%
	$(GLSLC) -o $@ $< $(SHADER_FLAGS)

shaders: vcpkg_installed_eval $(_TARGETS)


debug: init vcpkg_installed_eval shaders $(deb_objs) build_deb 
ifeq ($(OS),Windows_NT)
	.\client_deb
else
	./client_deb
endif
	
release: init vcpkg_installed_eval shaders lum-al/lib/liblumal.a build_rel 
ifeq ($(OS),Windows_NT)
	.\client_rel
else
	./client_rel
endif


lum-al/lib/liblumal.a: vcpkg_installed
	git submodule init
	git submodule update
	cd lum-al
	make library
	cd ..

#mostly for testing
only_build: init vcpkg_installed_eval shaders lum-al/lib/liblumal.a $(rel_objs) build_rel

#crazy fast
crazy: init vcpkg_installed_eval shaders
	$(CPP_COMPILER) $(srcs) -o crazy_client $(crazy_flags) $(I) $(L) $(REQUIRED_LIBS) $(STATIC_OR_DYNAMIC)
crazy_native: init vcpkg_installed_eval shaders
	$(CPP_COMPILER) $(srcs) -o crazy_client $(crazy_flags) -march=native $(I) $(L) $(REQUIRED_LIBS) $(STATIC_OR_DYNAMIC)

#i could not make it work without this
build_deb: setup $(deb_objs)
	$(CPP_COMPILER) -o client_deb $(deb_objs) $(debug_flags) $(I) $(L) $(REQUIRED_LIBS) $(STATIC_OR_DYNAMIC)
# strip .\client_deb.exe
build_rel: setup $(rel_objs) 
	$(CPP_COMPILER) -o client_rel $(rel_objs) $(release_flags) $(I) $(L) $(REQUIRED_LIBS) $(STATIC_OR_DYNAMIC)
# strip .\client_rel.exe

fun:
	@echo -e '\033[0;36m' fun was never an option '\033[0m'
test:
	$(CPP_COMPILER) -pg test.cpp -o test -Wl,--stack,1000000
	test

pack:
ifeq ($(OS),Windows_NT)
	mkdir "package"
	mkdir "package/shaders/compiled"
	mkdir "package/assets"
	copy "client" "package/client"
	copy "shaders/compiled" "package/shaders/compiled"
	copy "assets" "package/assets"
	powershell Compress-Archive -Update package package.zip
else
	mkdir -p package
	mkdir -p package/shaders/compiled
	mkdir -p package/assets
	cp client package/client
	cp -a /shaders/compiled /package/shaders/compiled
	cp -a /assets           /package/assets
	zip -r package.zip package
endif

# yeah windows wants quotes and backslashes so linux obviously wants no quotes and forward slashes. They have to be incompatible.
cleans: init
ifeq ($(OS),Windows_NT)
	del "shaders\compiled\*.spv" 
else
	rm -R shaders/compiled/*.spv
endif

cleand: init
ifeq ($(OS),Windows_NT)
	del "obj\deb\*.o" 
else
	rm -R obj/deb/*.o
endif

cleanr: init
ifeq ($(OS),Windows_NT)
	del "obj\rel\*.o"  
else
	rm -R obj/rel/*.o
endif

clean: init
ifeq ($(OS),Windows_NT)
	-del "obj\*.o"
	-del "obj\deb\*.o"
	-del "obj\deb\*.d"
	-del "obj\rel\*.o"
	-del "obj\rel\*.d"
	-del "shaders\compiled\*.spv"
else
	-rm -R obj/*.o
	-rm -R obj/deb/*.o 
	-rm -R obj/deb/*.d 
	-rm -R obj/rel/*.o 
	-rm -R obj/rel/*.d 
	-rm -R shaders/compiled/*.spv 
endif

# mkdir obj
init: obj obj/deb obj/deb/actors obj/rel obj/rel/actors shaders/compiled
obj:
ifeq ($(OS),Windows_NT)
	mkdir "obj"
else
	mkdir -p obj
endif

obj/deb:
ifeq ($(OS),Windows_NT)
	mkdir "obj/deb"
else
	mkdir -p obj/deb
endif

obj/deb/actors:
ifeq ($(OS),Windows_NT)
	mkdir "obj/deb/actors"
else
	mkdir -p obj/deb/actors
endif

obj/rel:
ifeq ($(OS),Windows_NT)
	mkdir "obj/rel"
else
	mkdir -p obj/rel
endif

obj/rel/actors:
ifeq ($(OS),Windows_NT)
	mkdir "obj/rel/actors"
else
	mkdir -p obj/rel/actors
endif

shaders/compiled:
ifeq ($(OS),Windows_NT)
	mkdir "shaders/compiled"
else
	mkdir -p shaders/compiled
endif

vcpkg_installed:
	echo installing vcpkg dependencies. Please do not interrupt
	vcpkg install

#use when big changes happen to lum 
update: 
	echo updating vcpkg dependencies. Please do not interrupt
	vcpkg install
	git submodule init
	git submodule update
	cd lum-al
	vcpkg install
	cd..
	cd box2d
	make