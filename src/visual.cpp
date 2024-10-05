#include "visual.hpp"
#include "defines/macros.hpp"
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
    for (mut fillerPipe : fillerPipes) {
        render.descriptorBuilder
            .setLayout(&fillerPipe.setLayout)
            .setDescriptorSets(&fillerPipe.sets)
            .setDescriptions({
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RD_CURRENT, &uniform, {/*empty*/}, NO_SAMPLER, NO_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT}
            })
            .defer();
    }

    render.descriptorBuilder
        .setLayout(&bloomApplyPipe.setLayout).setDescriptorSets(&bloomApplyPipe.sets)
        .setDescriptions({
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RD_CURRENT, &uniform, {/*empty*/}, NO_SAMPLER, NO_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, RD_CURRENT, {/*empty*/}, &frame, NO_SAMPLER, VK_IMAGE_LAYOUT_GENERAL, VK_SHADER_STAGE_FRAGMENT_BIT},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, RD_CURRENT, {/*empty*/}, &bloomExtracted, nearestSampler, VK_IMAGE_LAYOUT_GENERAL, VK_SHADER_STAGE_FRAGMENT_BIT},
        })
        .defer();
    render.descriptorBuilder
        .setLayout(&bloomExtractPipe.setLayout).setDescriptorSets(&bloomExtractPipe.sets)
        .setDescriptions({
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RD_CURRENT, &uniform, {/*empty*/}, NO_SAMPLER, NO_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, RD_CURRENT, {/*empty*/}, &frame, NO_SAMPLER, VK_IMAGE_LAYOUT_GENERAL, VK_SHADER_STAGE_FRAGMENT_BIT},
        })
        .defer();

    render.createDescriptorSetLayout ({
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT},
    }, &bloomUpsamplePushLayout,
    VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    render.createDescriptorSetLayout ({
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT},
    }, &bloomDownsamplePushLayout,
    VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);

    render.descriptorBuilder
        .setLayout(&bloomUpsamplePipe.setLayout).setDescriptorSets(&bloomUpsamplePipe.sets)
        .setDescriptions({
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RD_CURRENT, &uniform, {/*empty*/}, NO_SAMPLER, NO_LAYOUT, VK_SHADER_STAGE_COMPUTE_BIT},
        })
        .defer();
    render.descriptorBuilder
        .setLayout(&bloomDownsamplePipe.setLayout).setDescriptorSets(&bloomDownsamplePipe.sets)
        .setDescriptions({
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, RD_CURRENT, &uniform, {/*empty*/}, NO_SAMPLER, NO_LAYOUT, VK_SHADER_STAGE_COMPUTE_BIT},
        })
        .defer();
    // for(int mip=0; mip<bloomMimapped.current().mip_views.size(); mip++){
    // }

    render.flushDescriptorSetup();
}

void VisualView::createImages(void) {
    render.createImageStorages(&frame,
        VK_IMAGE_TYPE_2D, FRAME_FORMAT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        {render.swapChainExtent.width, render.swapChainExtent.height, 1});
        
    int mips = 
        log2(glm::max(render.swapChainExtent.width/2, render.swapChainExtent.height/2))+1;// - log2(40) + 1;
    render.createImageStorages(&bloomExtracted,
        VK_IMAGE_TYPE_2D, BLOOM_FORMAT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        {render.swapChainExtent.width, render.swapChainExtent.height,1}, u32(1));
    render.createImageStorages(&bloomMimapped,
        VK_IMAGE_TYPE_2D, BLOOM_FORMAT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        {render.swapChainExtent.width/2, render.swapChainExtent.height/2,1}, u32(mips));
    // render.transitionImageLayoutSingletime(&bloomMimapped[0], VK_IMAGE_LAYOUT_GENERAL, 9);
}

void VisualView::createPasses(void) {
    vector<RasterPipe*> pipes = {};
    for(mut pipe: fillerPipes){
        pipes.push_back(&pipe);
    }
    render.renderPassBuilder.setAttachments({
            {&frame,          Clear,Store, DontCare,DontCare, {.color= {.float32={0,0,0,1}}}, VK_IMAGE_LAYOUT_GENERAL},
            {&bloomExtracted, Clear,Store, DontCare,DontCare, {.color= {.float32={0,0,0,0}}}, VK_IMAGE_LAYOUT_GENERAL},
        }).setSubpasses({
            {pipes, {}, {&frame}, {}},
            {{&bloomExtractPipe}, {&frame}, {&bloomExtracted}, {}},
        })
        .build(&mainPass);

    //produces final swapchain image
    render.renderPassBuilder.setAttachments({
            {&render.swapchainImages, Clear,Store, DontCare,DontCare, {.color= {.float32={0,0,0,1}}}, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
            {&frame,           Load,DontCare, DontCare,DontCare, {.color= {.float32={0,0,0,0}}}, VK_IMAGE_LAYOUT_GENERAL},
            {&bloomExtracted,  Load,DontCare, DontCare,DontCare, {.color= {.float32={0,0,0,0}}}, VK_IMAGE_LAYOUT_GENERAL},
        }).setSubpasses({
            {{&bloomApplyPipe}, {&frame, &bloomExtracted}, {&render.swapchainImages}, {}},
        })
        .build(&bloomApplyPass);
}

void VisualView::createPipes(void) {
    //TODO lum-al reflection?
    createFillerPipes({
        {SolidColor, "shaders/compiled/solid.frag.spv"},
        {RandomColor, "shaders/compiled/random.frag.spv"}, 
        {LEDstyle, "shaders/compiled/led.frag.spv"}, 
        {FBMstyle, "shaders/compiled/fbm.frag.spv"}, 
        {GRIDstyle, "shaders/compiled/grid.frag.spv"}, 
        {WAVYstyle, "shaders/compiled/wavy.frag.spv"}, 
    });

    render.pipeBuilder.setStages({
            {"shaders/compiled/fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
            {"shaders/compiled/bloom_apply.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
        })
        .setAttributes({}).setCreateFlags(0).setStride(0)
        .setExtent(render.swapChainExtent)
        .setBlends({BLEND_MIX})
        .buildRaster(&bloomApplyPipe);
    render.pipeBuilder.setStages({
            {"shaders/compiled/fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
            {"shaders/compiled/bloom_extract.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
        })
        .setExtent(render.swapChainExtent)
        .setBlends({NO_BLEND})
        .buildRaster(&bloomExtractPipe);

    // render.pipeBuilder.setStages({
    //         {"shaders/compiled/bloom_down.frag.spv", VK_SHADER_STAGE_COMPUTE_BIT}
    //     })
    //     .buildCompute(&bloomDownsamplePipe);
    render.pipeBuilder.setStages({
            {"shaders/compiled/bloom_up.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT}
        }).setExtraDynamicLayout(bloomUpsamplePushLayout)
        .buildCompute(&bloomUpsamplePipe);
    render.pipeBuilder.setStages({
            {"shaders/compiled/bloom_down.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT}
        }).setExtraDynamicLayout(bloomDownsamplePushLayout)
        .buildCompute(&bloomDownsamplePipe);
}

void VisualView::createSamplers(void) {
    // render.createSampler(&linearSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    render.createSampler(&nearestSampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.maxAnisotropy = 1.0;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.maxAnisotropy = 8.0f; //safe i guess
		samplerInfo.anisotropyEnable = VK_FALSE;
    VK_CHECK(vkCreateSampler(render.device, &samplerInfo, nullptr, &linearSampler));
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
    for(mut shapes : static_shape_vectors){
        shapes.clear();}
}
void VisualView::reset_dynamic_shapes(void){
    // Map itself is untouched
    // Vectors that store "drawn" shapes are cleared 
    for(mut shapes : dynamic_shape_vectors){
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
    render.cmdNextSubpass(graphicsCommandBuffers.current(), &mainPass);
    render.cmdBindPipe(graphicsCommandBuffers.current(), bloomExtractPipe);
    render.cmdDraw(graphicsCommandBuffers.current(), 3, 1, 0, 0);

    render.cmdEndRenderPass(graphicsCommandBuffers.current(), &mainPass);
// println
}

void VisualView::mipmap_bloom(void) {
    // Generate mipmaps and apply bloom
}

void VisualView::bloom_pass(void) {
    //downsampling extracted, blit to mipmaps to start processing
    // render.blitWholeImage(graphicsCommandBuffers.current(), bloomExtracted.current(), bloomMimapped.current(), VK_FILTER_LINEAR);
    int w = bloomMimapped.current().extent.width;
    int h = bloomMimapped.current().extent.height;
    int mips = bloomMimapped.current().mip_levels;
    // pl(mips);
    // render.generateMipmaps(graphicsCommandBuffers.current(), bloomMimapped.current().image, w, h, mips, VK_IMAGE_ASPECT_COLOR_BIT);
    
    {
        //TODO: move
        VkDescriptorImageInfo
            lower_res = {};
            lower_res.imageView = bloomMimapped.current().mip_views[0];
            lower_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDescriptorImageInfo
            higher_res = {};
            higher_res.imageView = bloomExtracted.current().view;
            higher_res.sampler = linearSampler;
            higher_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet
            low_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            low_write.dstSet = NULL;
            low_write.dstBinding = 0;
            low_write.dstArrayElement = 0;
            low_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            low_write.descriptorCount = 1;
            low_write.pImageInfo = &lower_res;
        VkWriteDescriptorSet
            high_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            high_write.dstSet = NULL;
            high_write.dstBinding = 1;
            high_write.dstArrayElement = 0;
            high_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            high_write.descriptorCount = 1;
            high_write.pImageInfo = &higher_res;
        vector<VkWriteDescriptorSet> descriptorWrites = {low_write, high_write};
        vkCmdPushDescriptorSetKHR (graphicsCommandBuffers.current(), VK_PIPELINE_BIND_POINT_COMPUTE, 
            bloomDownsamplePipe.lineLayout, 1, descriptorWrites.size(), descriptorWrites.data());

        render.cmdBindPipe(graphicsCommandBuffers.current(), bloomDownsamplePipe);
        int mip_w = bloomExtracted.current().extent.width / 2;
        int mip_h = bloomExtracted.current().extent.height / 2;
        render.cmdDispatch(graphicsCommandBuffers.current(), mip_w/8, mip_h/8, 1);
    }

    for(int mip=0; mip<mips-1; mip++){
        // al();
        VkDescriptorImageInfo
            lower_res = {};
            lower_res.imageView = bloomMimapped.current().mip_views[mip+1];
            lower_res.sampler = linearSampler;
            lower_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDescriptorImageInfo
            higher_res = {};
            higher_res.imageView = bloomMimapped.current().mip_views[mip];
            higher_res.sampler = linearSampler;
            higher_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet
            low_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            low_write.dstSet = NULL;
            low_write.dstBinding = 0;
            low_write.dstArrayElement = 0;
            low_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            low_write.descriptorCount = 1;
            low_write.pImageInfo = &lower_res;
        VkWriteDescriptorSet
            high_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            high_write.dstSet = NULL;
            high_write.dstBinding = 1;
            high_write.dstArrayElement = 0;
            high_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            high_write.descriptorCount = 1;
            high_write.pImageInfo = &higher_res;
        vector<VkWriteDescriptorSet> descriptorWrites = {low_write, high_write};
        vkCmdPushDescriptorSetKHR (graphicsCommandBuffers.current(), VK_PIPELINE_BIND_POINT_COMPUTE, 
            bloomDownsamplePipe.lineLayout, 1, descriptorWrites.size(), descriptorWrites.data());

        render.cmdBindPipe(graphicsCommandBuffers.current(), bloomDownsamplePipe);
        int mip_w = w / (1 << (mip+1));
        int mip_h = h / (1 << (mip+1));
        render.cmdDispatch(graphicsCommandBuffers.current(), mip_w/8, mip_h/8, 1);
    }


    //upsampling extracted
    for(int mip=mips-1; mip>0; mip--){
        VkDescriptorImageInfo
            lower_res = {};
            lower_res.imageView = bloomMimapped.current().mip_views[mip];
            lower_res.sampler = linearSampler;
            lower_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDescriptorImageInfo
            higher_res = {};
            // higher_res.sampler = linearSampler;
            higher_res.imageView = bloomMimapped.current().mip_views[mip-1];
            higher_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet
            low_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            low_write.dstSet = NULL;
            low_write.dstBinding = 0;
            low_write.dstArrayElement = 0;
            low_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            low_write.descriptorCount = 1;
            low_write.pImageInfo = &lower_res;
        VkWriteDescriptorSet
            high_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            high_write.dstSet = NULL;
            high_write.dstBinding = 1;
            high_write.dstArrayElement = 0;
            high_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            high_write.descriptorCount = 1;
            high_write.pImageInfo = &higher_res;
        vector<VkWriteDescriptorSet> descriptorWrites = {low_write, high_write};
        vkCmdPushDescriptorSetKHR (graphicsCommandBuffers.current(), VK_PIPELINE_BIND_POINT_COMPUTE, 
            bloomUpsamplePipe.lineLayout, 1, descriptorWrites.size(), descriptorWrites.data());

        render.cmdBindPipe(graphicsCommandBuffers.current(), bloomUpsamplePipe);
        int mip_w = w / (1 << (mip-1));
        int mip_h = h / (1 << (mip-1));
        render.cmdDispatch(graphicsCommandBuffers.current(), mip_w/8, mip_h/8, 1);
    }
    {
        //TODO: move
        //one more! With final, full-res blur in extracted
        VkDescriptorImageInfo
            lower_res = {};
            lower_res.imageView = bloomMimapped.current().mip_views[0];
            lower_res.sampler = linearSampler;
            lower_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDescriptorImageInfo
            higher_res = {};
            higher_res.imageView = bloomExtracted.current().view;
            higher_res.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet
            low_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            low_write.dstSet = NULL;
            low_write.dstBinding = 0;
            low_write.dstArrayElement = 0;
            low_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            low_write.descriptorCount = 1;
            low_write.pImageInfo = &lower_res;
        VkWriteDescriptorSet
            high_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            high_write.dstSet = NULL;
            high_write.dstBinding = 1;
            high_write.dstArrayElement = 0;
            high_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            high_write.descriptorCount = 1;
            high_write.pImageInfo = &higher_res;
        vector<VkWriteDescriptorSet> descriptorWrites = {low_write, high_write};
        vkCmdPushDescriptorSetKHR (graphicsCommandBuffers.current(), VK_PIPELINE_BIND_POINT_COMPUTE, 
            bloomUpsamplePipe.lineLayout, 1, descriptorWrites.size(), descriptorWrites.data());

        render.cmdBindPipe(graphicsCommandBuffers.current(), bloomUpsamplePipe);
        int mip_w = bloomExtracted.current().extent.width;
        int mip_h = bloomExtracted.current().extent.height;
        render.cmdDispatch(graphicsCommandBuffers.current(), mip_w/8, mip_h/8, 1);
    }
    render.cmdBeginRenderPass(graphicsCommandBuffers.current(), &bloomApplyPass);
    render.cmdBindPipe(graphicsCommandBuffers.current(), bloomApplyPipe);
    render.cmdDraw(graphicsCommandBuffers.current(), 3, 1, 0, 0);
    render.cmdEndRenderPass(graphicsCommandBuffers.current(), &bloomApplyPass);
}

void VisualView::end_frame(void) {
    render.end_frame({graphicsCommandBuffers.current()});
    
    graphicsCommandBuffers.move();
    copyCommandBuffers.move();
    staticShapeBuffer.move();
    dynamicShapeBuffer.move();
    
    frame.move();
    bloomExtracted.move();
    bloomMimapped.move();
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
    for (let shapes : static_shape_vectors) {
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
        for (let shapes : dynamic_shape_vectors) {
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
    for (let [coloringType, fragmentShaderFile] : fshaderFiles) {
        RasterPipe& pipe = fillerPipes[coloringType];
        render.pipeBuilder.setStages({
            {"shaders/compiled/default.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
            {fragmentShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT}
            }).setExtent(render.swapChainExtent).setBlends({BLEND_MIX})
            .setAttributes({
                {VK_FORMAT_R8G8B8_UINT, offsetof (Shape, coloring_info)},
                {VK_FORMAT_R8_UINT, offsetof (Shape, shapeType)},
                {VK_FORMAT_R32G32_SFLOAT, offsetof (Shape, pos)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, rot_angle)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, rounding_radius)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, props.value_1)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, props.value_2)},
                {VK_FORMAT_R32_SFLOAT, offsetof (Shape, props.value_3)},
            }).setStride(sizeof(Shape)).setPushConstantSize(0)
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