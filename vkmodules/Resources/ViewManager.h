#ifndef VKMODULES_VIEW_MANAGER_H
#define VKMODULES_VIEW_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_ImageView etc.

// Function to create a Vulkan image view
VkResult createImageView(
    VkDevice logicalDevice,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    VK_ImageView* imageViewData, // Output struct
    std::vector<VkResult>* vkres);

#endif // VKMODULES_VIEW_MANAGER_H