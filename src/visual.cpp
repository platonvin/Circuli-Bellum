#include "visual.hpp"
#include <string.h>

#define println printf("%s:%d: Fun: %s\n", __FILE__, __LINE__, __FUNCTION__);

void VisualView::setup() {
    Settings settings = {};
        settings.vsync = false; //every time deciding to which image to render, wait until monitor draws current. Icreases perfomance, but limits fps
        settings.fullscreen = false;
        settings.debug = true; //Validation Layers. Use them while developing or be tricked into thinking that your code is correct
        settings.timestampCount = 128;
        settings.profile = false; //monitors perfomance via timestamps. You can place one with PLACE_TIMESTAMP() macro
        settings.fif = 2; // Frames In Flight. If 1, then record cmdbuff and submit it. If multiple, cpu will (might) be ahead of gpu by FIF-1, which makes GPU wait less

        settings.deviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
        settings.deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
        settings.deviceFeatures.geometryShader = VK_TRUE;
        settings.deviceFeatures.independentBlend = VK_TRUE;
        settings.deviceFeatures.shaderInt16 = VK_TRUE;
        settings.deviceFeatures11.storagePushConstant16 = VK_TRUE;
        settings.deviceFeatures12.storagePushConstant8 = VK_TRUE;
        settings.deviceFeatures12.shaderInt8 = VK_TRUE;
        settings.deviceFeatures12.storageBuffer8BitAccess = VK_TRUE;
    render.init(settings);

    render.createCommandBuffers(&graphicsCommandBuffers, settings.fif);
    render.createCommandBuffers(&copyCommandBuffers, settings.fif);

    createShapeBuffers();    
    createUniformBuffer();    
    createSamplers();

    // Initialize swapchain dependent resources
    createSwapchainDependent();

    render.createSwapchainDependent = createSwapchainDependent;
    render.cleanupSwapchainDependent = cleanupSwapchainDependent;
}

void VisualView::cleanup(void) {
    // Because there are frames in flight
    render.deviceWaitIdle();
    cleanupSwapchainDependent();
    render.deleteBuffers(&uniform);
    render.deleteBuffers(&staticShapeBuffer);
    render.deleteBuffers(&dynamicShapeBuffer);
    render.cleanup();
}

void VisualView::setupDescriptors(void) {
    for (/*mutable*/ let fillerPipe : fillerPipes) {
        render.descriptorBuilder
            .setLayout(&fillerPipe.setLayout)
            .setDescriptorSets(&fillerPipe.sets)
            .setDescriptions({
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RD_CURRENT, &uniform, {/*empty*/}, NO_SAMPLER, NO_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT}
            })
            .defer();
    }

    // render.descriptorBuilder
    //     .setLayout(&bloomPipe.setLayout)
    //     .setDescriptorSets(&bloomPipe.sets)
    //     .defer();

    render.flushDescriptorSetup();
}

void VisualView::createImages(void) {
    render.createImageStorages(&frame,
        VK_IMAGE_TYPE_2D, FRAME_FORMAT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        {render.swapChainExtent.width, render.swapChainExtent.height, 1});
        
    render.createImageStorages(&maskFrame,
        VK_IMAGE_TYPE_2D, FRAME_FORMAT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        {render.swapChainExtent.width / 2, render.swapChainExtent.height / 2, 1});
}

void VisualView::createPasses(void) {
    vector<RasterPipe*> pipes = {};
    for(let pipe: fillerPipes){
        pipes.push_back(&pipe);
    }
    render.renderPassBuilder.setAttachments({
            {&render.swapchainImages, Clear,Store, DontCare,DontCare, {}, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
                         //pipes, no input, output to swapchain, no depth
        }).setSubpasses({{pipes, {}, {&render.swapchainImages}, {}}}).build(&mainPass);

    //TODO BLOOM
}

void VisualView::createPipes(void) {
    createFillerPipes({
        {SolidColor, "shaders/compiled/solid.frag.spv"},
        {RandomColor, "shaders/compiled/random.frag.spv"}, 
        {LEDstyle, "shaders/compiled/led.frag.spv"}, 
        {FBMstyle, "shaders/compiled/fbm.frag.spv"}, 
        {GRIDstyle, "shaders/compiled/grid.frag.spv"}, 
    });

    // render.pipeBuilder.setStages({
    //         {"shaders/compiled/fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
    //         {"shaders/compiled/bloom.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
    //     })
    //     .setExtent(render.swapChainExtent)
    //     .setBlends({BLEND_MIX})
    //     .buildRaster(&bloomPipe);
}

void VisualView::createSamplers(void) {
    // Set up samplers as needed
}

void VisualView::start_frame(void) {
    render.start_frame({graphicsCommandBuffers.current()});
    updateUniformBuffers();
}

void VisualView::start_main_pass(void) {
    render.cmdBeginRenderPass(graphicsCommandBuffers.current(), &mainPass);
    reset_dynamic_shapes();
}

void VisualView::reset_static_shapes(void){
    // Map itself is untouched
    // Vectors that store "drawn" shapes are cleared 
    for(let shapes : static_shape_vectors){
        shapes.clear();}
}
void VisualView::reset_dynamic_shapes(void){
    // Map itself is untouched
    // Vectors that store "drawn" shapes are cleared 
    for(let shapes : dynamic_shape_vectors){
        shapes.clear();}
}
void VisualView::draw_static_shape(Shape shape, ColoringType type) {
    static_shape_vectors[type].push_back(shape);}
void VisualView::draw_dynamic_shape(Shape shape, ColoringType type){
    dynamic_shape_vectors[type].push_back(shape);}

void VisualView::end_main_pass(void) {
    // Now, given shapes in vector, we need to send them to GPU and render
// println
    copyDynamicShapesToGPU();
// println
    drawShapes();
// println

    render.cmdEndRenderPass(graphicsCommandBuffers.current(), &mainPass);
// println
}

void VisualView::mipmap_bloom(void) {
    // Generate mipmaps and apply bloom
}

void VisualView::bloom_pass(void) {
    render.cmdBeginRenderPass(graphicsCommandBuffers.current(), &bloomPass);
    render.cmdBindPipe(graphicsCommandBuffers.current(), bloomPipe);
    render.cmdDraw(graphicsCommandBuffers.current(), 3, 1, 0, 0);
    render.cmdEndRenderPass(graphicsCommandBuffers.current(), &bloomPass);
}

void VisualView::end_frame(void) {
    render.end_frame({graphicsCommandBuffers.current()});
    
    graphicsCommandBuffers.move();
    copyCommandBuffers.move();
    frame.move();
    staticShapeBuffer.move();
    dynamicShapeBuffer.move();
}

void VisualView::createShapeBuffers(){
    VkDeviceSize 
        bufferSize = sizeof(Shape) * max_static_shape_count;
    render.createBufferStorages(&staticShapeBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, true);
    render.mapBufferStorages(&staticShapeBuffer);
        bufferSize = sizeof(Shape) * max_dynamic_shape_count;
    render.createBufferStorages(&dynamicShapeBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, true);
    render.mapBufferStorages(&dynamicShapeBuffer);
}
void VisualView::createUniformBuffer(){
    VkDeviceSize 
        bufferSize = 28;
    render.createBufferStorages(&uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, true);
    render.mapBufferStorages(&uniform);
}

void VisualView::copyStaticShapesToGPU(){
    VkDeviceSize offset = 0;
    for (const let shapes : static_shape_vectors) {
        VkDeviceSize size = sizeof(Shape) * shapes.size();
        assert(offset + size <= sizeof(Shape) * max_static_shape_count);

        memcpy((char*)staticShapeBuffer.current().mapped + offset, shapes.data(), size);
        offset += size;
    }
}
void VisualView::copyDynamicShapesToGPU(){
// println
        VkDeviceSize offset = 0;
// println
        for (const let shapes : dynamic_shape_vectors) {
            VkDeviceSize size = sizeof(Shape) * shapes.size();
            if(size > 0){
                assert(offset + size <= sizeof(Shape) * max_dynamic_shape_count);
                dynamicShapeBuffer.current();

                memcpy((char*)dynamicShapeBuffer.current().mapped + offset, shapes.data(), size);
                offset += size;
            }
        }
}

void VisualView::createFillerPipes(vector<std::pair<ColoringType, const char*>> fshaderFiles){
    //so they can be in any order
    for (const let [coloringType, fragmentShaderFile] : fshaderFiles) {
        RasterPipe& pipe = fillerPipes[coloringType];
        render.pipeBuilder.setStages({
            {"shaders/compiled/default.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
            {fragmentShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT}
            }).setExtent(render.swapChainExtent).setBlends({BLEND_MIX})
            .setAttributes({
                {VK_FORMAT_R8G8B8_UINT, offsetof (Shape, coloring_info)},
                {VK_FORMAT_R8_UINT, offsetof (Shape, shapeType)},
                {VK_FORMAT_R32G32_SFLOAT, offsetof (Shape, pos)},
                {VK_FORMAT_R32G32_SFLOAT, offsetof (Shape, rot)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, props.value_1)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, props.value_2)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, props.value_3)},
            }).setStride(sizeof(Shape)).setPushConstantSize(0)
            .setExtent(render.swapChainExtent).setBlends({BLEND_MIX})
            .setCulling(VK_CULL_MODE_NONE).setInputRate(VK_VERTEX_INPUT_RATE_INSTANCE).setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .buildRaster(&pipe);
    }
}
void VisualView::updateUniformBuffers(){
    struct {vec2 pos, size; ivec2 res; float time;} unicopy = {
        camera.cameraPos, camera.cameraScale,
        ivec2(render.swapChainExtent.width, render.swapChainExtent.height), float(glfwGetTime())};
    vkCmdUpdateBuffer(graphicsCommandBuffers.current(), uniform.current().buffer, 0, sizeof(unicopy), &unicopy);    
    render.cmdPipelineBarrier (graphicsCommandBuffers.current(),
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT|VK_ACCESS_TRANSFER_WRITE_BIT,
        uniform.current());   
}

void VisualView::drawShapes() {
    VkDeviceSize staticOffset = 0;
    VkDeviceSize dynamicOffset = 0;
    VkCommandBuffer commandBuffer = graphicsCommandBuffers.current();
    // Draw corresponding shapes for each pair of {coloringType, pipe}
    // Total ColoringType's * 2 drawcalls
    for (int coloringType=0; coloringType<COLORING_TYPE_SIZE; coloringType++) {
        render.cmdBindPipe(commandBuffer, fillerPipes[coloringType]);

        // Draw static shapes
        let staticShapes = static_shape_vectors[coloringType];
        if (staticShapes.size() > 0) {
            render.cmdBindVertexBuffers(commandBuffer, 0, 1, &staticShapeBuffer.current().buffer, &staticOffset);
            render.cmdDraw(commandBuffer, 6, staticShapes.size(), 0, 0);
            staticOffset += sizeof(Shape) * staticShapes.size();
        }

        // Draw dynamic shapes
        let dynamicShapes = dynamic_shape_vectors[coloringType];
        if (dynamicShapes.size() > 0) {
            render.cmdBindVertexBuffers(commandBuffer, 0, 1, &dynamicShapeBuffer.current().buffer, &dynamicOffset);
            render.cmdDraw(commandBuffer, 6, dynamicShapes.size(), 0, 0);
            dynamicOffset += sizeof(Shape) * dynamicShapes.size();
        }
    }
}