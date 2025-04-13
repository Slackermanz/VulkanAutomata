#ifndef VKMODULES_BUFFER_MANAGER_H
#define VKMODULES_BUFFER_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_Buffer_Data etc.
#include "../VulkanFoundation/VulkanCore.h" // For findProperties

// Function to create a Vulkan buffer, allocate memory, and bind it
VkResult createBuffer(
    VkDevice logicalDevice,
    VkPhysicalDevice physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryProperties,
    VkSharingMode sharingMode,
    const uint32_t* pQueueFamilyIndices,
    uint32_t queueFamilyIndexCount,
    VK_Buffer_Data* bufferData, // Using VK_Buffer_Data to store results
    std::vector<VkResult>* vkres);

#endif // VKMODULES_BUFFER_MANAGER_H