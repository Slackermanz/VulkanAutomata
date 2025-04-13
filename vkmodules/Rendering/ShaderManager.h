#ifndef VKMODULES_SHADER_MANAGER_H
#define VKMODULES_SHADER_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "../Types/VulkanTypes.h" // For ShaderData

// Function to load shader code from a file and create a VkShaderModule
VkResult loadAndCreateShaderModule(
    VkDevice logicalDevice,
    const std::string& filename,
    ShaderData* shaderData, // Output struct (stores module handle and info)
    std::vector<VkResult>* vkres
);

#endif // VKMODULES_SHADER_MANAGER_H