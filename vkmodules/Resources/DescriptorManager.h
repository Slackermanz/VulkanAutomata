#ifndef VKMODULES_DESCRIPTOR_MANAGER_H
#define VKMODULES_DESCRIPTOR_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_DescSetLayout3, VK_ImageView etc.

// Function to create descriptor set layout, pool, and allocate sets
// This specifically handles the layout with 3 bindings (Sampler, UBO, SSBO) used in the work pipeline
VkResult createWorkDescriptorSet(
    VkDevice logicalDevice,
    VK_DescSetLayout3* descSetLayoutData, // Input/Output struct
    uint32_t setCount, // Number of sets to allocate (usually 2 for ping-pong)
    std::vector<VkResult>* vkres
);

// Function to update the descriptor sets with buffer and image info
void updateWorkDescriptorSets(
    VkDevice logicalDevice,
    VK_DescSetLayout3* descSetLayoutData, // Contains allocated sets
    VkSampler sampler,
    VK_ImageView work_init[2], // *** CORRECTED: Pass array of VK_ImageView structs ***
    VkBuffer uniformBuffer,
    VkBuffer storageBuffer,
    uint32_t setCount // Should match allocation count (usually 2)
);

#endif // VKMODULES_DESCRIPTOR_MANAGER_H