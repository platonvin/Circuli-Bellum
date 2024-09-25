#pragma once

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
};

typedef struct Shape {
    u8 coloringType;
    u8vec3 color; //solid color.
    vec2 pos;

    u8 shapeType;
    union {
        struct{
            float CIRCLE_radius;
        };
        struct{
            float SQUARE_width;
            float SQUARE_height;
        };
        struct{
            float CAPSULE_radius;
            float CAPSULE_length;
        };
    };
    // TODO: char size?
} Shape;

// Why runtime? For later runtime mods
class VisualView{
public:
    Renderer render = {};
    const int max_static_shape_count = 1024;
    const int max_dynamic_shape_count = 1024;
    // Created on level setup
    // vector<Shape> static_shapes = {};
    std::unordered_map<u8, vector<Shape>> static_shapes;
    // Resets every frame
    // vector<Shape> dynamic_shapes = {};
    std::unordered_map<u8, vector<Shape>> dynamic_shapes;

    vector<std::pair<u8, RasterPipe>> fillerPipes;

    Camera camera = {};

    // Store a pair of RasterPipe and coloringType
    void init (Settings settings);
    void cleanup();
    void setupDescriptors();
    void createImages();
    void createSwapchainDependentImages();
    void cleanupSwapchainDependent();
    void createPipilines();
    void createSamplers();

    void createShapeBuffers();
        void copyStaticShapesToGPU();
        void copyDynamicShapesToGPU();
        // void sortStaticShapesByColoringType();
        // void sortDynamicShapesByColoringType();
    void createFillerPipes(const vector<std::pair<u8, const char*>> shaderFiles);
    void updateUniformBuffers();
    void drawShapes();
    // sets voxels and size. By default uses first .vox palette as main palette

    void reset_static_shapes();
        void draw_static_shape(Shape);

    void start_frame();
        void start_main_pass();
            void draw_shape(Shape);
        void end_main_pass();
        void mipmap_bloom();
        void bloom_pass();
    void end_frame();

    ring<Buffer> staticShapeBuffer;
    ring<Buffer> dynamicShapeBuffer;

    RasterPipe bloomPipe;

    RenderPass mainPass; //draws everything
    RenderPass bloomPass; //bloom!

    ring<VkCommandBuffer> graphicsCommandBuffers;
    ring<VkCommandBuffer> copyCommandBuffers; //runtime copies for ui. Also does first frame resources

    ring<Image> frame;
    ring<Image> maskFrame; //where lowres renders to. Blends with highres afterwards
    ring<Buffer> uniform;

  private:
    const VkFormat FRAME_FORMAT = VK_FORMAT_R16G16B16A16_UNORM;
};