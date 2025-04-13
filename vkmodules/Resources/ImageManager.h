#ifndef VKMODULES_IMAGE_MANAGER_H
#define VKMODULES_IMAGE_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_Layer_1x2D etc.
#include "../VulkanFoundation/VulkanCore.h" // For findProperties

// Function to create a Vulkan image, allocate memory, and bind it
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
    VK_Layer_1x2D* imageData, // Using VK_Layer_1x2D to store results
    std::vector<VkResult>* vkres);

#endif // VKMODULES_IMAGE_MANAGER_H