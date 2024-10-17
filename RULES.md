### C++ rules:
 - no logic in constructors / destructors (exception: isolated class); TODO: remove all, including default values
 - less std templates (freezes clangd lol). Write yourself (maybe from preprocessed std) //TODO modules? TODO? lowe std version?
 - no useless functions (return m_var) (~= no getters/setters)
 - TODO:? zero as valid value for malloc/memset
 - alloca()? 
 - no useless const / constexpr. If need perfomance - build as single TU (unity build)
 - use #pragma once
 - vec4 > vec3? TODO: test
 - no ECS before 2.0 era // for a measurement