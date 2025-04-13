#ifndef VKMODULES_RENDER_PASS_MANAGER_H
#define VKMODULES_RENDER_PASS_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_RenderPass etc.

// Function to create a simple Vulkan render pass with one color attachment
VkResult createSimpleColorRenderPass(
    VkDevice logicalDevice,
    VkFormat format,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout,
    VK_RenderPass* renderPassData, // Output struct
    std::vector<VkResult>* vkres);

#endif // VKMODULES_RENDER_PASS_MANAGER_H