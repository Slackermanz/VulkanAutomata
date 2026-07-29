#include "CommandBufferManager.h"

// Note: Includes for Logger are now handled via CommandBufferManager.h

VkResult createCommandBuffers(
    VkDevice device,
    uint32_t queueFamilyIndex, 
    uint32_t count,
    VK_Command* commandBuffers,
    std::vector<VkResult>* vkres) {
    
    for(uint32_t i = 0; i < count; i++) {
        // Create command pool
        commandBuffers[i].pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandBuffers[i].pool_info.pNext = NULL;
        commandBuffers[i].pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandBuffers[i].pool_info.queueFamilyIndex = queueFamilyIndex;
        
        VkResult result = vkCreateCommandPool(device, &commandBuffers[i].pool_info, NULL, &commandBuffers[i].vk_command_pool);
        vr("vkCreateCommandPool", vkres, commandBuffers[i].vk_command_pool, result);
        if(result != VK_SUCCESS) { return result; }
        
        // Allocate command buffer
        commandBuffers[i].comm_buff_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBuffers[i].comm_buff_alloc_info.pNext = NULL;
        commandBuffers[i].comm_buff_alloc_info.commandPool = commandBuffers[i].vk_command_pool;
        commandBuffers[i].comm_buff_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBuffers[i].comm_buff_alloc_info.commandBufferCount = 1;
        
        result = vkAllocateCommandBuffers(device, &commandBuffers[i].comm_buff_alloc_info, &commandBuffers[i].vk_command_buffer);
        vr("vkAllocateCommandBuffers", vkres, commandBuffers[i].vk_command_buffer, result);
        if(result != VK_SUCCESS) {
            vkDestroyCommandPool(device, commandBuffers[i].vk_command_pool, NULL);
            commandBuffers[i].vk_command_pool = VK_NULL_HANDLE;
            return result;
        }
        
        // Setup command buffer begin info
        commandBuffers[i].comm_buff_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        nf(&commandBuffers[i].comm_buff_begin_info);
        commandBuffers[i].comm_buff_begin_info.pInheritanceInfo = NULL;
    }

    return VK_SUCCESS;
}