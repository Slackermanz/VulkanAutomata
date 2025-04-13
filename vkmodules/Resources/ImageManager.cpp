#include "ImageManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, ov, iv, nf)

VkResult createImage(
    VkDevice logicalDevice,
    VkPhysicalDevice physicalDevice,
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryProperties,
    VkSharingMode sharingMode,
    const uint32_t* pQueueFamilyIndices,
    uint32_t queueFamilyIndexCount,
    VK_Layer_1x2D* imageData, // Out parameter
    std::vector<VkResult>* vkres) {

    // 1. Setup Extent
    imageData->ext3D.width = width;
    imageData->ext3D.height = height;
    imageData->ext3D.depth = 1;

    // 2. Create Image Info
    imageData->img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    nf(&imageData->img_info); // Set pNext to NULL and flags to 0
    imageData->img_info.imageType = VK_IMAGE_TYPE_2D;
    imageData->img_info.format = format;
    imageData->img_info.extent = imageData->ext3D;
    imageData->img_info.mipLevels = 1;
    imageData->img_info.arrayLayers = 1;
    imageData->img_info.samples = VK_SAMPLE_COUNT_1_BIT;
    imageData->img_info.tiling = tiling;
    imageData->img_info.usage = usage;
    imageData->img_info.sharingMode = sharingMode;
    imageData->img_info.queueFamilyIndexCount = queueFamilyIndexCount;
    imageData->img_info.pQueueFamilyIndices = pQueueFamilyIndices;
    imageData->img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Common initial layout

    // 3. Create Image
    VkResult result = vkCreateImage(logicalDevice, &imageData->img_info, NULL, &imageData->vk_image);
    vr("vkCreateImage", vkres, imageData->vk_image, result);
    if (result != VK_SUCCESS) {
        return result; // Failed to create image
    }

    // 4. Get Memory Requirements
    rv("vkGetImageMemoryRequirements");
    vkGetImageMemoryRequirements(logicalDevice, imageData->vk_image, &imageData->vk_mem_reqs);
    ov("Image memreq size", imageData->vk_mem_reqs.size);
    ov("Image memreq alignment", imageData->vk_mem_reqs.alignment);
    ov("Image memreq memoryTypeBits", imageData->vk_mem_reqs.memoryTypeBits);

    // 5. Find Memory Type Index
    VkPhysicalDeviceMemoryProperties pdevMemProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &pdevMemProps);
    imageData->MTB_index = findProperties(
        &pdevMemProps,
        imageData->vk_mem_reqs.memoryTypeBits,
        memoryProperties);
    ov("Image memoryTypeIndex", imageData->MTB_index);
    if (imageData->MTB_index == -1) {
        // Handle error: suitable memory type not found
        vkDestroyImage(logicalDevice, imageData->vk_image, NULL);
        return VK_ERROR_INITIALIZATION_FAILED; // Or a more specific error
    }

    // 6. Allocate Memory Info
    imageData->vk_mem_allo_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageData->vk_mem_allo_info.pNext = NULL;
    imageData->vk_mem_allo_info.allocationSize = imageData->vk_mem_reqs.size;
    imageData->vk_mem_allo_info.memoryTypeIndex = imageData->MTB_index;

    // 7. Allocate Memory
    result = vkAllocateMemory(logicalDevice, &imageData->vk_mem_allo_info, NULL, &imageData->vk_dev_mem);
    vr("vkAllocateMemory", vkres, imageData->vk_dev_mem, result);
    if (result != VK_SUCCESS) {
        // Clean up created image before returning
        vkDestroyImage(logicalDevice, imageData->vk_image, NULL);
        return result; // Failed to allocate memory
    }

    // 8. Bind Image Memory
    result = vkBindImageMemory(logicalDevice, imageData->vk_image, imageData->vk_dev_mem, 0);
    vr("vkBindImageMemory", vkres, imageData->vk_image, result);
    if (result != VK_SUCCESS) {
        // Clean up allocated memory and created image
        vkFreeMemory(logicalDevice, imageData->vk_dev_mem, NULL);
        vkDestroyImage(logicalDevice, imageData->vk_image, NULL);
        return result; // Failed to bind memory
    }

    return VK_SUCCESS; // Image created, memory allocated and bound successfully
}