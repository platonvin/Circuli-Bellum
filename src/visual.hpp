#pragma once

#define GLFW_INCLUDE_NONE
#include "al.hpp"

typedef struct Camera {
    vec2 cameraPos = vec2(0, 0);
    vec2 cameraScale = vec2(1, 1);
    //TODO damper?
} Camera;

enum ShapeType{
    Circle,
    Square,
    Capsule,
};

enum ColoringType{
    SolidColor,
    RandomColor,
    COLORING_TYPE_SIZE,
};

typedef struct Shape {
    u8vec3 coloring_info; // Solid color AND anything else for non-solid-color coloringType
    u8 shapeType;
    vec2 pos;

    union {
        struct{
            float CIRCLE_radius;
        };
        struct{
            float SQUARE_half_width;
            float SQUARE_half_height;
        };
        struct{
            float CAPSULE_radius;
            float CAPSULE_half_length;
        };
        struct{ // for attributes
            float value_1;
            float value_2;
        };
    };
    // TODO: char size?
} Shape;

// TODO runtime? For later runtime mods
class VisualView{
public:
    Renderer render = {};
    const int max_static_shape_count = 1024;
    const int max_dynamic_shape_count = 1024;
    // Created on level setup
    // vector<Shape> static_shapes = {};
    std::array<vector<Shape>, COLORING_TYPE_SIZE> static_shape_vectors;
    // Resets every frame
    // vector<Shape> dynamic_shapes = {};
    std::array<vector<Shape>, COLORING_TYPE_SIZE> dynamic_shape_vectors;

    std::array<RasterPipe, COLORING_TYPE_SIZE> fillerPipes;

    Camera camera = {};

    // Store a pair of RasterPipe and coloringType
    void init ();
    void cleanup();
    void createImages();
    void createSwapchainDependentImages();
    void cleanupSwapchainDependent();
    void setupDescriptors();
    void createPasses();
    void createPipes();
    void createSamplers();
    void createFillerPipes(const vector<std::pair<u8, const char*>> shaderFiles);
    void createShapeBuffers();
    void createUniformBuffer();

        void copyStaticShapesToGPU();
        void copyDynamicShapesToGPU();
        // void sortStaticShapesByColoringType();
        // void sortDynamicShapesByColoringType();
    void updateUniformBuffers();
    void drawShapes();
    // sets voxels and size. By default uses first .vox palette as main palette

    void reset_static_shapes();
        void draw_static_shape(Shape, ColoringType);

    void reset_dynamic_shapes();
    void start_frame();
        void start_main_pass();
            void draw_dynamic_shape(Shape, ColoringType);
        void end_main_pass();
        void mipmap_bloom();
        void bloom_pass();
    void end_frame();

    ring<Buffer> staticShapeBuffer;
    ring<Buffer> dynamicShapeBuffer;

    RasterPipe bloomPipe;

    RenderPass mainPass; //draws everything
    RenderPass bloomPass; //bloom!
    //TODO BLOOM

    ring<VkCommandBuffer> graphicsCommandBuffers;
    ring<VkCommandBuffer> copyCommandBuffers; //runtime copies for ui. Also does first frame resources

    ring<Image> frame;
    ring<Image> maskFrame;
    ring<Buffer> uniform;

  private:
    const VkFormat FRAME_FORMAT = VK_FORMAT_R16G16B16A16_UNORM;
};