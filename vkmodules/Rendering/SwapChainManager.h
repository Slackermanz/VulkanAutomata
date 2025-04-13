#ifndef VKMODULES_SWAPCHAIN_MANAGER_H
#define VKMODULES_SWAPCHAIN_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_Obj etc.

// Function to create the Vulkan swapchain
VkResult createSwapChain(
    VK_Obj* vob,                      // Core Vulkan objects (for VKL, VKI, VKP)
    VkSurfaceKHR surface,             // Window surface
    VkSurfaceCapabilitiesKHR* capabilities, // Surface capabilities
    uint32_t queueFamilyIndex,        // Graphics queue family index
    VkSwapchainKHR* oldSwapchain,     // For recreation (optional, VK_NULL_HANDLE for initial)
    VkSwapchainKHR* swapChain,        // Output: Swapchain handle
    uint32_t* imageCount,             // Output: Number of images in swapchain
    std::vector<VkImage>* swapChainImages, // Output: Vector to store swapchain image handles
    std::vector<VkResult>* vkres
);

#endif // VKMODULES_SWAPCHAIN_MANAGER_H