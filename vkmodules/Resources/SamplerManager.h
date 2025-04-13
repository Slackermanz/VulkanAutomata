#ifndef VKMODULES_SAMPLER_MANAGER_H
#define VKMODULES_SAMPLER_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_RPConfig potentially, though not strictly needed for output

// Function to create a Vulkan sampler with common settings used in the project
VkResult createDefaultSampler(
    VkDevice logicalDevice,
    VkSampler* sampler, // Output sampler handle
    std::vector<VkResult>* vkres
);

#endif // VKMODULES_SAMPLER_MANAGER_H