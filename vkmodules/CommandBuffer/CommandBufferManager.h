#ifndef VKMODULES_COMMAND_BUFFER_MANAGER_H
#define VKMODULES_COMMAND_BUFFER_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h"
#include "../Utils/Logger.h" // Needed for vr, rv, nf used within createCommandBuffers

// Create a set of command buffers (pool and allocation)
VkResult createCommandBuffers(
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t count,
    VK_Command* commandBuffers,
    std::vector<VkResult>* vkres);

#endif // VKMODULES_COMMAND_BUFFER_MANAGER_H