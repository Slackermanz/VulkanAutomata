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

// Enumerate and select a suitable physical device
VkResult selectPhysicalDevice(
    VkInstance instance,
    VK_Obj* vob, // Output: Stores selected device handle and index
    VK_PhysDev* selectedPdevInfo, // Output: Stores properties/features of selected device
    std::vector<VkResult>* vkres
);

// Find a queue family supporting graphics operations
VkResult findGraphicsQueueFamily(
    VkPhysicalDevice physicalDevice,
    uint32_t* queueFamilyIndex, // Output: The found queue family index
    uint32_t* queueCount,      // Output: The number of queues in the family
    std::vector<VkResult>* vkres
);

// Setup the device queue create info structure
void setupDeviceQueueCreateInfo(
    uint32_t queueFamilyIndex,
    uint32_t queueCount,
    const float* pQueuePriorities, // Pointer to array of priorities
    VK_PDQueues* pdq // Output struct
);

// Create the logical device
VkResult createLogicalDevice(
    VkPhysicalDevice physicalDevice,
    VK_PDQueues* pdq,               // Contains queue create info
    const char** device_extensions, // Device extensions to enable
    uint32_t device_extension_count,
    VkPhysicalDeviceFeatures* enabledFeatures, // Features to enable
    VK_Obj* vob,                    // Output: Stores logical device handle (VKL)
    std::vector<VkResult>* vkres
);


#endif // VKMODULES_VULKAN_CORE_H