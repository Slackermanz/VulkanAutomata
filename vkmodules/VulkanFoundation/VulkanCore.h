#ifndef VKMODULES_VULKAN_CORE_H
#define VKMODULES_VULKAN_CORE_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "../Types/VulkanTypes.h"

// Debug callback function for Vulkan validation layers
VkBool32 debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

// Find a memory type that meets the requirements
int32_t findProperties(
    const VkPhysicalDeviceMemoryProperties* pMemoryProperties,
    uint32_t memoryTypeBitsRequirement,
    VkMemoryPropertyFlags requiredProperties);

// Initialize Vulkan instance
VkResult initVulkanInstance(
    VK_Obj* vob,
    VK_Config* vkcfg,
    const char** instance_extensions,
    uint32_t instance_extension_count,
    const char** layer_extensions,
    uint32_t layer_extension_count,
    std::vector<VkResult>* vkres);

// Setup debug messenger
VkResult setupDebugMessenger(
    VK_Obj* vob,
    VK_Debug* vkdbg,
    std::vector<VkResult>* vkres);

// Initialize required GLFW extensions
bool initGLFWExtensions(
    uint32_t* glfw_ext_count,
    const char*** glfw_extensions,
    std::vector<VkResult>* vkres);

#endif // VKMODULES_VULKAN_CORE_H