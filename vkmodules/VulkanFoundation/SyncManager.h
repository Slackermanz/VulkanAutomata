#ifndef VKMODULES_SYNC_MANAGER_H
#define VKMODULES_SYNC_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>

// Function to create a Vulkan semaphore
VkResult createSemaphore(
    VkDevice logicalDevice,
    VkSemaphore* semaphore, // Output handle
    std::vector<VkResult>* vkres
);

// Function to create a Vulkan fence
VkResult createFence(
    VkDevice logicalDevice,
    VkFenceCreateFlags flags, // e.g., VK_FENCE_CREATE_SIGNALED_BIT or 0
    VkFence* fence,           // Output handle
    std::vector<VkResult>* vkres
);

#endif // VKMODULES_SYNC_MANAGER_H