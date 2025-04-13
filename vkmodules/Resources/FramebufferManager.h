#ifndef VKMODULES_FRAMEBUFFER_MANAGER_H
#define VKMODULES_FRAMEBUFFER_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_FrameBuff etc.

// Function to create a Vulkan framebuffer
VkResult createFramebuffer(
    VkDevice logicalDevice,
    VkRenderPass renderPass,
    VkImageView imageView,
    uint32_t width,
    uint32_t height,
    VK_FrameBuff* framebufferData, // Output struct
    std::vector<VkResult>* vkres);

#endif // VKMODULES_FRAMEBUFFER_MANAGER_H