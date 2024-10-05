#pragma once
#include "macros.hpp"
#ifndef __VISUAL_HPP__
#define __VISUAL_HPP__

// #define GLFW_INCLUDE_NONE
#include "al.hpp"

typedef struct Camera {
    vec2 cameraPos = vec2(0, 0);
    vec2 cameraScale = vec2(1, 1);
    //TODO damper?
} Camera;

enum ShapeType : unsigned char{
    Circle,
    Square,
    Capsule,
    Trapezoid,
};

//TODO state change is neccessary? or depth? 
enum ColoringType : unsigned char{
    WAVYstyle,
    SolidColor,
    RandomColor,
    FBMstyle,
    LEDstyle,
    GRIDstyle,
    COLORING_TYPE_SIZE,
};

typedef union ShapeProps {
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
    struct{
        float TRAPEZOID_half_bottom_size;
        float TRAPEZOID_half_top_size;
        float TRAPEZOID_half_height;
    };
    struct{ // for attributes
        float value_1 = 69.421; // for easy debugging
        float value_2 = 69.422; // for easy debugging
        float value_3 = 69.423; // for easy debugging
    };
} ShapeProps;

typedef struct Shape {
    u8vec3 coloring_info; // Solid color AND anything else for non-solid-color coloringType
    ShapeType shapeType;
    vec2 pos;
    // vec2 rot = vec2(1,0);
    float rot_angle = 0;
    float rounding_radius = 0;

    ShapeProps props;
    // TODO: char size?
} Shape;

// TODO runtime? For later runtime mods
class VisualView{
public:
    Renderer render = {};
    //TODO?
    const int max_static_shape_count = 1 << 16;
    const int max_dynamic_shape_count = 1 << 16;
    // Created on level setup
    std::array<vector<Shape>, COLORING_TYPE_SIZE> static_shape_vectors;
    // Resets every frame
    std::array<vector<Shape>, COLORING_TYPE_SIZE> dynamic_shape_vectors;

    std::array<RasterPipe, COLORING_TYPE_SIZE> fillerPipes;

    Camera camera = {};

    // Store a pair of RasterPipe and coloringType
    void setup ();
    void cleanup();
    void createImages();
    void createSwapchainDependentImages();
    // void cleanupSwapchainDependent();
    void setupDescriptors();
    void createPasses();
    void createPipes();
    void createSamplers();
    void createFillerPipes(const vector<std::pair<ColoringType, const char*>> shaderFiles);
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

    //subpass of main
    RasterPipe bloomExtractPipe;
    //compute shaders
    VkDescriptorSetLayout bloomDownsamplePushLayout;
    ComputePipe bloomDownsamplePipe; // operates on bloom mips
    // vector<ring<VkDescriptorSet>> bloomUpDsets;
    VkDescriptorSetLayout bloomUpsamplePushLayout;
    ComputePipe bloomUpsamplePipe; // operates on bloom mips
    //for second pass
    RasterPipe bloomApplyPipe; // blends bloom with original

    RenderPass mainPass; //draws everything and extracts bloom
    RenderPass bloomApplyPass;
    //TODO BLOOM

    ring<VkCommandBuffer> graphicsCommandBuffers;
    ring<VkCommandBuffer> copyCommandBuffers; //runtime copies for ui. Also does first frame resources

    ring<Image> frame;
    ring<Image> bloomExtracted; // w,h;
    ring<Image> bloomMimapped; // w,h / 2 -> 1
    ring<Buffer> uniform;

  private:
    // const VkFormat FRAME_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat FRAME_FORMAT = VK_FORMAT_R16G16B16A16_UNORM;
    const VkFormat BLOOM_FORMAT = VK_FORMAT_R16G16B16A16_UNORM;
    VkSampler linearSampler;
    VkSampler nearestSampler;

    std::function<VkResult(void)> createSwapchainDependent = [this](){
        graphicsCommandBuffers = {};
        copyCommandBuffers = {};
        for(mut frp : fillerPipes){
            frp = {};
            l();
        }
        
        render.createCommandBuffers(&graphicsCommandBuffers, render.settings.fif);
        render.createCommandBuffers(&copyCommandBuffers, render.settings.fif);

        // Initialize other resources
        createImages();
        // createSamplers();
        setupDescriptors(); // Only after buffers
        createPasses(); // Only after descriptors
        createPipes(); // Only after rpasses

        //you have to set this if you want to use builtin profiler
        render.mainCommandBuffers = &graphicsCommandBuffers;

        return VK_SUCCESS;
    };

    std::function<VkResult(void)> cleanupSwapchainDependent = [this](){
        render.deviceWaitIdle();
        
        // render.deleteImages(&frame);
        render.deleteImages(&bloomMimapped);
        
        render.destroyRenderPass(&mainPass);
        for(mut frp : fillerPipes){
            render.destroyRasterPipeline(&frp);
        }
        // render.destroyRasterPipeline(&bloomPipe);
        return VK_SUCCESS;
    };
};

#endif // __VISUAL_HPP__