#ifndef VKMODULES_SYNC_MANAGER_H
#define VKMODULES_SYNC_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h" // For VK_QueueSync

// Function to get the device queue handle
void getDeviceQueue(
    VkDevice logicalDevice,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex, // Usually 0
    VkQueue* queue       // Output queue handle
);

// Function to setup a basic VkSubmitInfo structure (no semaphores)
void setupBasicSubmitInfo(
    VK_QueueSync* qSync // Output struct
);

// Function to setup VkSubmitInfo for presentation loop (waits on image acquire)
void setupPresentSubmitInfo(
    VkSemaphore* pWaitSemaphore, // *** CHANGED: Pointer to wait semaphore ***
    VkPipelineStageFlags* pWaitStageMask, // *** CHANGED: Pointer to wait stage mask ***
    VkSemaphore* pSignalSemaphore, // *** CHANGED: Pointer to signal semaphore ***
    VK_QueueSync* qSync // Output struct
);

// Function to setup VkSubmitInfo for ImGui rendering (waits on present loop)
void setupImGuiSubmitInfo(
    VkSemaphore* pWaitSemaphore, // *** CHANGED: Pointer to wait semaphore ***
    VkPipelineStageFlags* pWaitStageMask, // *** CHANGED: Pointer to wait stage mask ***
    VkSemaphore* pSignalSemaphore, // *** CHANGED: Pointer to signal semaphore ***
    VK_QueueSync* qSync // Output struct
);

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