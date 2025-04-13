#include "CommandBufferManager.h"
#include "../Utils/Logger.h"

void createCommandBuffers(
    VkDevice device,
    uint32_t queueFamilyIndex, 
    uint32_t count,
    VK_Command* commandBuffers,
    std::vector<VkResult>* vkres) {
    
    for(int i = 0; i < count; i++) {
        // Create command pool
        commandBuffers[i].pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandBuffers[i].pool_info.pNext = NULL;
        commandBuffers[i].pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandBuffers[i].pool_info.queueFamilyIndex = queueFamilyIndex;
        
        vr("vkCreateCommandPool", vkres, commandBuffers[i].vk_command_pool,
            vkCreateCommandPool(device, &commandBuffers[i].pool_info, NULL, &commandBuffers[i].vk_command_pool));
        
        // Allocate command buffer
        commandBuffers[i].comm_buff_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBuffers[i].comm_buff_alloc_info.pNext = NULL;
        commandBuffers[i].comm_buff_alloc_info.commandPool = commandBuffers[i].vk_command_pool;
        commandBuffers[i].comm_buff_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBuffers[i].comm_buff_alloc_info.commandBufferCount = 1;
        
        vr("vkAllocateCommandBuffers", vkres, commandBuffers[i].vk_command_buffer,
            vkAllocateCommandBuffers(device, &commandBuffers[i].comm_buff_alloc_info, &commandBuffers[i].vk_command_buffer));
        
        // Setup command buffer begin info
        commandBuffers[i].comm_buff_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        nf(&commandBuffers[i].comm_buff_begin_info);
        commandBuffers[i].comm_buff_begin_info.pInheritanceInfo = NULL;
    }
}

void recordWorkInitCommands(
    VK_Command* commandBuffer,
    VK_ImageView* workInit,
    int index,
    std::vector<VkResult>* vkres) {
    
    vr("vkBeginCommandBuffer", vkres, index,
        vkBeginCommandBuffer(commandBuffer->vk_command_buffer, &commandBuffer->comm_buff_begin_info));
    
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, &workInit->img_mem_barr);
    
    vr("vkEndCommandBuffer", vkres, index,
        vkEndCommandBuffer(commandBuffer->vk_command_buffer));
}

void recordWorkLoopCommands(
    VK_Command* commandBuffer,
    VkRenderPassBeginInfo* renderPassInfo,
    VK_DescSetLayout3* descriptorSetLayout,
    VK_Pipe* pipeline,
    int index,
    std::vector<VkResult>* vkres) {
    
    vr("vkBeginCommandBuffer", vkres, index,
        vkBeginCommandBuffer(commandBuffer->vk_command_buffer, &commandBuffer->comm_buff_begin_info));
    
    // Begin render pass
    rv("vkCmdBeginRenderPass");
        vkCmdBeginRenderPass(
            commandBuffer->vk_command_buffer, renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Bind pipeline and descriptor sets
    rv("vkCmdBindPipeline");
        vkCmdBindPipeline(
            commandBuffer->vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline);
    
    rv("vkCmdBindDescriptorSets");
        vkCmdBindDescriptorSets(
            commandBuffer->vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vk_pipeline_layout,
            0, 1, &descriptorSetLayout->vk_descriptor_set[index], 0, NULL);
    
    // Draw call
    rv("vkCmdDraw");
        vkCmdDraw(
            commandBuffer->vk_command_buffer, 3, 1, 0, 0);
    
    // End render pass
    rv("vkCmdEndRenderPass");
        vkCmdEndRenderPass(commandBuffer->vk_command_buffer);
    
    vr("vkEndCommandBuffer", vkres, index,
        vkEndCommandBuffer(commandBuffer->vk_command_buffer));
}

void recordPresentationCommands(
    VK_Command* commandBuffer,
    VK_Layer_2x2D* workImages,
    VkImage* swapchainImages,
    VkImageMemoryBarrier* workBarriers,
    VkImageMemoryBarrier* presentBarriers,
    VkImageBlit* imageBlit,
    uint32_t swapchainIndex,
    uint32_t workIndex,
    uint32_t swapchainCount,
    std::vector<VkResult>* vkres) {
    
    int i = swapchainIndex + (workIndex * swapchainCount);
    
    vr("vkBeginCommandBuffer", vkres, i,
        vkBeginCommandBuffer(commandBuffer->vk_command_buffer, &commandBuffer->comm_buff_begin_info));
    
    // Transition swapchain image to transfer destination layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, &presentBarriers[0]);
    
    // Transition work image to transfer source layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, &workBarriers[0]);
    
    // Blit the work image to the swapchain image
    rv("vkCmdBlitImage");
        vkCmdBlitImage(
            commandBuffer->vk_command_buffer, 
            workImages->vk_image[workIndex], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchainImages[swapchainIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, imageBlit, VK_FILTER_NEAREST);
    
    // Transition work image back to shader read layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, &workBarriers[1]);
    
    // Transition swapchain image to present layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, &presentBarriers[1]);
    
    vr("vkEndCommandBuffer", vkres, i,
        vkEndCommandBuffer(commandBuffer->vk_command_buffer));
}

void recordImageExportCommands(
    VK_Command* commandBuffer,
    VK_Layer_2x2D* workImages,
    VK_Layer_1x2D* blitImage,
    VkImageMemoryBarrier* workToTransferBarrier,
    VkImageMemoryBarrier* workToShaderBarrier,
    VkImageBlit* imageBlit,
    int index,
    std::vector<VkResult>* vkres) {
    
    vr("vkBeginCommandBuffer", vkres, index,
        vkBeginCommandBuffer(commandBuffer->vk_command_buffer, &commandBuffer->comm_buff_begin_info));
    
    // Transition work image to transfer source layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, workToTransferBarrier);
    
    // Blit work image to export image
    rv("vkCmdBlitImage");
        vkCmdBlitImage(
            commandBuffer->vk_command_buffer, 
            workImages->vk_image[index], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            blitImage->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, imageBlit, VK_FILTER_NEAREST);
    
    // Transition work image back to shader read layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, workToShaderBarrier);
    
    vr("vkEndCommandBuffer", vkres, index,
        vkEndCommandBuffer(commandBuffer->vk_command_buffer));
}

void recordBufferToImageCommands(
    VK_Command* commandBuffer,
    VK_Layer_1x2D* blitImage,
    VK_Buffer_1x2D* buffer,
    VkImageMemoryBarrier* imageBarrierToSrc,
    VkImageMemoryBarrier* imageBarrierToDst,
    VkBufferImageCopy* copyRegion,
    std::vector<VkResult>* vkres) {
    
    vr("vkBeginCommandBuffer", vkres, 0,
        vkBeginCommandBuffer(commandBuffer->vk_command_buffer, &commandBuffer->comm_buff_begin_info));
    
    // Transition image to transfer source layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, imageBarrierToSrc);
    
    // Copy image to buffer
    rv("vkCmdCopyImageToBuffer");
        vkCmdCopyImageToBuffer(
            commandBuffer->vk_command_buffer, 
            blitImage->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            buffer->vk_buffer,
            1, copyRegion);
    
    // Transition image back to transfer destination layout
    rv("vkCmdPipelineBarrier");
        vkCmdPipelineBarrier(
            commandBuffer->vk_command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, NULL, 0, NULL,
            1, imageBarrierToDst);
    
    vr("vkEndCommandBuffer", vkres, 0,
        vkEndCommandBuffer(commandBuffer->vk_command_buffer));
}

VkResult submitCommandBuffer(
    VkDevice device,
    VkQueue queue,
    VkSubmitInfo* submitInfo,
    VkCommandBuffer* commandBuffer,
    VkFence fence,
    bool waitForCompletion,
    std::vector<VkResult>* vkres) {
    
    submitInfo->pCommandBuffers = commandBuffer;
    
    VkResult result = vkQueueSubmit(queue, 1, submitInfo, fence);
    vr("vkQueueSubmit", vkres, *commandBuffer, result);
    
    // Wait for completion if requested
    if (waitForCompletion && fence != VK_NULL_HANDLE) {
        do {
            result = vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000);
            vr("vkWaitForFences <100ms>", vkres, fence, result);
        } while (result == VK_TIMEOUT);
    }
    
    return result;
}