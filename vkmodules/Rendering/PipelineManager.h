#ifndef VKMODULES_PIPELINE_MANAGER_H
#define VKMODULES_PIPELINE_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_Pipe, VK_PipeInfo etc.

// Function to create a pipeline layout
VkResult createPipelineLayout(
    VkDevice logicalDevice,
    VkDescriptorSetLayout descriptorSetLayout, // Assumes single descriptor set layout
    VK_Pipe* pipelineData, // Output struct (stores layout handle)
    std::vector<VkResult>* vkres
);

// Function to create a graphics pipeline (specifically for the 'Work' pipeline)
VkResult createWorkGraphicsPipeline(
    VkDevice logicalDevice,
    VK_Pipe* pipelineData, // Input: contains layout handle. Output: stores pipeline handle.
    VK_PipeInfo* pipelineConfig, // Contains rasterization, viewport, blend, shader stages etc.
    VkRenderPass renderPass, // The render pass the pipeline will be used with
    std::vector<VkResult>* vkres
);

#endif // VKMODULES_PIPELINE_MANAGER_H