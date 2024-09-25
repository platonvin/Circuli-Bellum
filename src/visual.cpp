#include "visual.hpp"
#include <string.h>

void VisualView::init(Settings settings) {
    render.init(settings);

    // Create command buffers
    render.createCommandBuffers(&graphicsCommandBuffers, settings.fif);
    render.createCommandBuffers(&copyCommandBuffers, settings.fif);

    // Initialize other resources
    createImages();
    createSamplers();
    setupDescriptors();
    createPipilines();
}

void VisualView::cleanup(void) {
    // Because there are frames in flight
    render.deviceWaitIdle();
    cleanupSwapchainDependent();
    render.deleteBuffers(&uniform);
    render.cleanup();
}

void VisualView::setupDescriptors(void) {
    render.descriptorBuilder
        .setLayout(&shapesPipe.setLayout)
        .setDescriptorSets(&shapesPipe.sets)
        .defer();

    render.descriptorBuilder
        .setLayout(&bloomPipe.setLayout)
        .setDescriptorSets(&bloomPipe.sets)
        .defer();

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

void VisualView::createSwapchainDependentImages(void) {
    createImages();
}
void VisualView::cleanupSwapchainDependent(void) {
    render.deleteImages(&frame);
    render.deleteImages(&maskFrame);
}

void VisualView::createPipilines(void) {
    render.pipeBuilder.setStages({
            {"shaders/shapes.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
            {"shaders/shapes.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
        })
        .setExtent(render.swapChainExtent)
        .setBlends({BLEND_MIX})
        .buildRaster(&shapesPipe);

    render.pipeBuilder.setStages({
            {"shaders/bloom.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
            {"shaders/bloom.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
        })
        .setExtent(render.swapChainExtent)
        .setBlends({BLEND_MIX})
        .buildRaster(&bloomPipe);
}

void VisualView::createSamplers(void) {
    // Set up samplers as needed
}

void VisualView::start_frame(void) {
    render.start_frame({graphicsCommandBuffers.current()});
}

void VisualView::start_main_pass(void) {
    render.cmdBeginRenderPass(graphicsCommandBuffers.current(), &mainPass);
    dynamic_shapes.clear();
}

void VisualView::reset_static_shapes(void){static_shapes.clear();}
void VisualView::draw_static_shape(Shape shape) {static_shapes.push_back(shape);}

void VisualView::draw_shape(Shape shape){dynamic_shapes.push_back(shape);}

void VisualView::end_main_pass(void) {
    // Now, given shapes in vector, we need to prepare them, send to GPU and render

    // for()
    
    render.cmdEndRenderPass(graphicsCommandBuffers.current(), &mainPass);
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
}

void VisualView::createShapeBuffers(){
    VkDeviceSize 
        bufferSize = sizeof(Shape) * max_static_shape_count;
    render.createBufferStorages(&staticShapeBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, true);
    render.mapBufferStorages(staticShapeBuffer);
        bufferSize = sizeof(Shape) * max_dynamic_shape_count;
    render.createBufferStorages(&dynamicShapeBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT, bufferSize, true);
    render.mapBufferStorages(dynamicShapeBuffer);
}
void VisualView::copyStaticShapesToGPU(){
    VkDeviceSize offset = 0;
    for (const auto& [coloringType, shapes] : static_shapes) {
        VkDeviceSize size = sizeof(Shape) * shapes.size();
        assert(offset + size <= sizeof(Shape) * max_static_shape_count);

        // Copy shapes into the mapped buffer with the appropriate offset
        memcpy(((char*)staticShapeBuffer.current().mapped + offset), shapes.data(), size);
        offset += size;
    }
}
void VisualView::copyDynamicShapesToGPU(){
        VkDeviceSize offset = 0;
        for (const auto& [coloringType, shapes] : dynamic_shapes) {
            VkDeviceSize size = sizeof(Shape) * shapes.size();
            assert(offset + size <= sizeof(Shape) * max_dynamic_shape_count);

            // Copy shapes into the mapped buffer with the appropriate offset
            memcpy(dynamicShapeBuffer.current().mapped + offset, shapes.data(), size);
            offset += size;
        }
}

void VisualView::createFillerPipes(vector<std::pair<u8, const char*>> fshaderFiles){
        for (const auto& [coloringType, fragmentShaderFile] : fshaderFiles) {
            RasterPipe pipe;
            render.pipeBuilder.setStages({
                {"shaders/compiled/default.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
                {fragmentShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT}
                }).setExtent(render.swapChainExtent).setBlends({BLEND_MIX})
                .buildRaster(&pipe);
            fillerPipes.emplace_back(coloringType, pipe);
        }
}
void VisualView::updateUniformBuffers(){

}
void VisualView::drawShapes() {
    VkDeviceSize offsets[] = {0};
    VkDeviceSize staticOffset = 0;
    VkDeviceSize dynamicOffset = 0;
    VkCommandBuffer commandBuffer = graphicsCommandBuffers.current();

    // Draw corresponding shapes for each pair of {coloringType, pipe}
    for (const auto& [coloringType, pipe] : fillerPipes) {
        render.cmdBindPipe(commandBuffer, pipe);

        // Draw static shapes
        if (static_shapes.count(coloringType) > 0) {
            auto& currentShapes = static_shapes[coloringType];

            render.cmdBindVertexBuffers(commandBuffer, 0, 1, &staticShapeBuffer.current().buffer, &staticOffset);
            render.cmdDraw(commandBuffer, 6, currentShapes.size(), 0, 0);
            staticOffset += sizeof(Shape) * currentShapes.size();
        }

        // Draw dynamic shapes
        if (dynamic_shapes.count(coloringType) > 0) {
            auto& currentShapes = dynamic_shapes[coloringType];

            render.cmdBindVertexBuffers(commandBuffer, 0, 1, &dynamicShapeBuffer.current().buffer, &dynamicOffset);
            render.cmdDraw(commandBuffer, 6, currentShapes.size(), 0, 0);
            dynamicOffset += sizeof(Shape) * currentShapes.size();
        }
    }
}