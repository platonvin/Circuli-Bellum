#pragma once
#include "macros.hpp"
#ifndef __VISUAL_HPP__
#define __VISUAL_HPP__

// #define GLFW_INCLUDE_NONE
#define GLM_FORCE_SWIZZLE
#include "al.hpp"

typedef struct Camera {
    vec2 cameraPos = vec2(0, 0);
    vec2 cameraScale = vec2(1, 1);
    //TODO damper?
} Camera;

enum ShapeType : unsigned char{
    Circle,
    Rectangle,
    Capsule,
    Trapezoid,
};

//TODO state change is neccessary? or depth? 
enum ColoringType : unsigned char{
    HealingFieldStyle, 
    SawFieldStyle, 
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
        float RECTANGLE_half_width;
        float RECTANGLE_half_height;
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
    u8vec4 coloring_info; // Solid color AND anything else for non-solid-color coloringType
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
    //do not need a lot
    const int max_shadow_shape_count = 1 << 10;
    //do need a lot. TODO? estimate max
    const int max_dynamic_shape_count = 1 << 18;
    // Created on level setup
    vector<Shape> shadow_shapes;
    // Resets every frame
    std::array<vector<Shape>, COLORING_TYPE_SIZE> dynamic_shape_vectors;

    std::array<RasterPipe, COLORING_TYPE_SIZE> filler_pipes;

    Shape background;
    ColoringType background_ct;
    VkDeviceSize background_offset;

    Camera camera = {};
    struct {
        vec2 pos; 
        vec2 size; 
        ivec2 res; 
        vec2 chrom_abb;
        vec2 light_pos;
        float time;
    } ubo_cpu;

    // Store a pair of RasterPipe and coloringType
    void setup ();
    void cleanup();
    void createImages();
    // void createSwapchainDependentImages();
    // void cleanupSwapchainDependent();
    void setupDescriptors();
    void createPasses();
    void createPipes();
    void createSamplers();
    void createFillerPipes(const vector<std::pair<ColoringType, const char*>> shaderFiles);
    void createShapeBuffers();
    void createUniformBuffer();

        void copyShadowShapesToGPU();
        void copyDynamicShapesToGPU();
        // void sortStaticShapesByColoringType();
        // void sortDynamicShapesByColoringType();
    void updateUniformBuffers();
    void drawShapes();
    // sets voxels and size. By default uses first .vox palette as main palette

    void reset_shadow_shapes();

    void reset_dynamic_shapes();
    void start_frame();
        void start_main_pass();
            void draw_background(Shape shape, ColoringType type);
            void draw_shadow_shape(Shape);
            void draw_dynamic_shape(Shape, ColoringType);
        void end_main_pass();
        void mipmap_bloom();
        void bloom_pass();
    void end_frame();

    ring<Buffer> shadowShapeBuffer;
    ring<Buffer> dynamicShapeBuffer; // last elem is background

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

    RasterPipe shadowPipe; // blends bloom with original
    RasterPipe shadowApplyPipe; // blends bloom with original

    RenderPass mainPass; //draws everything and extracts bloom
    RenderPass bloomApplyPass;
    RenderPass shadowPass; // 1d rasterizetion
    //TODO BLOOM

    ring<VkCommandBuffer> graphicsCommandBuffers;
    ring<VkCommandBuffer> copyCommandBuffers; //runtime copies for ui. Also does first frame resources

    ring<Image> frame;
    ring<Image> bloomExtracted; // w,h;
    ring<Image> bloomMimapped; // w,h / 2 -> 1
    ring<Buffer> uniform;
    ring<Image> shadowmap; // 1d texture for shadows from ONE source

  private:
    // const VkFormat FRAME_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat  FRAME_FORMAT = VK_FORMAT_R16G16B16A16_UNORM;
    const VkFormat  BLOOM_FORMAT = VK_FORMAT_R16G16B16A16_UNORM;
    const VkFormat SHADOW_FORMAT = VK_FORMAT_D32_SFLOAT; //TODO:
    const u32 SHADOWMAP_SIZE = 1024*16;
    VkSampler linearSampler;
    VkSampler nearestSampler;

    std::function<VkResult(void)> createSwapchainDependent = [this](){
        graphicsCommandBuffers = {};
        copyCommandBuffers = {};
        for(mut frp : filler_pipes){
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
        
        render.deleteImages(&frame);
        render.deleteImages(&shadowmap);
        render.deleteImages(&bloomExtracted); // w,h;
        render.deleteImages(&bloomMimapped); // w,h / 2 -> 1

        render.destroyRenderPass(&mainPass);
        render.destroyRenderPass(&bloomApplyPass);
        render.destroyRenderPass(&shadowPass);

        for(mut frp : filler_pipes){
            render.destroyRasterPipeline(&frp);
        }
        render.destroyRasterPipeline(&bloomApplyPipe);
        render.destroyRasterPipeline(&bloomExtractPipe);
        render.destroyComputePipeline(&bloomDownsamplePipe);
        render.destroyComputePipeline(&bloomUpsamplePipe);
        render.destroyRasterPipeline(&shadowPipe);
        render.destroyRasterPipeline(&shadowApplyPipe);
        // render.destroyRasterPipeline(&bloomPipe);
        vkDestroyDescriptorSetLayout(render.device, bloomDownsamplePushLayout, NULL);
        vkDestroyDescriptorSetLayout(render.device, bloomUpsamplePushLayout, NULL);
        return VK_SUCCESS;
    };
};

#endif // __VISUAL_HPP__