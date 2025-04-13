#include "BufferManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, ov, iv, nf)

VkResult createBuffer(
    VkDevice logicalDevice,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryProperties,
    VkSharingMode sharingMode,
    const uint32_t* pQueueFamilyIndices,
    uint32_t queueFamilyIndexCount,
    VK_Buffer_Data* bufferData, // Out parameter
    std::vector<VkResult>* vkres) {

    // 1. Create Buffer Info
    bufferData->buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    nf(&bufferData->buff_info); // Set pNext to NULL and flags to 0
    bufferData->buff_info.size = size;
    bufferData->buff_info.usage = usage;
    bufferData->buff_info.sharingMode = sharingMode;
    bufferData->buff_info.queueFamilyIndexCount = queueFamilyIndexCount;
    bufferData->buff_info.pQueueFamilyIndices = pQueueFamilyIndices;

    // 2. Create Buffer
    VkResult result = vkCreateBuffer(logicalDevice, &bufferData->buff_info, NULL, &bufferData->vk_buffer);
    vr("vkCreateBuffer", vkres, bufferData->vk_buffer, result);
    if (result != VK_SUCCESS) {
        return result; // Failed to create buffer
    }

    // 3. Get Memory Requirements
    rv("vkGetBufferMemoryRequirements");
    vkGetBufferMemoryRequirements(logicalDevice, bufferData->vk_buffer, &bufferData->vk_mem_reqs);
    ov("Buffer memreq size", bufferData->vk_mem_reqs.size);
    ov("Buffer memreq alignment", bufferData->vk_mem_reqs.alignment);
    ov("Buffer memreq memoryTypeBits", bufferData->vk_mem_reqs.memoryTypeBits);

    // 4. Find Memory Type Index
    VkPhysicalDeviceMemoryProperties pdevMemProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &pdevMemProps);
    bufferData->MTB_index = findProperties(
        &pdevMemProps,
        bufferData->vk_mem_reqs.memoryTypeBits,
        memoryProperties);
    ov("Buffer memoryTypeIndex", bufferData->MTB_index);
    if (bufferData->MTB_index == -1) {
        // Handle error: suitable memory type not found
        // Clean up created buffer before returning? Or let caller handle?
        return VK_ERROR_INITIALIZATION_FAILED; // Or a more specific error
    }

    // 5. Allocate Memory Info
    bufferData->vk_mem_allo_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    bufferData->vk_mem_allo_info.pNext = NULL;
    bufferData->vk_mem_allo_info.allocationSize = bufferData->vk_mem_reqs.size;
    bufferData->vk_mem_allo_info.memoryTypeIndex = bufferData->MTB_index;

    // 6. Allocate Memory
    result = vkAllocateMemory(logicalDevice, &bufferData->vk_mem_allo_info, NULL, &bufferData->vk_dev_mem);
    vr("vkAllocateMemory", vkres, bufferData->vk_dev_mem, result);
    if (result != VK_SUCCESS) {
        // Clean up created buffer before returning
        vkDestroyBuffer(logicalDevice, bufferData->vk_buffer, NULL);
        return result; // Failed to allocate memory
    }

    // 7. Bind Buffer Memory
    result = vkBindBufferMemory(logicalDevice, bufferData->vk_buffer, bufferData->vk_dev_mem, 0);
    vr("vkBindBufferMemory", vkres, bufferData->vk_buffer, result);
    if (result != VK_SUCCESS) {
        // Clean up allocated memory and created buffer
        vkFreeMemory(logicalDevice, bufferData->vk_dev_mem, NULL);
        vkDestroyBuffer(logicalDevice, bufferData->vk_buffer, NULL);
        return result; // Failed to bind memory
    }

    return VK_SUCCESS; // Buffer created, memory allocated and bound successfully
}