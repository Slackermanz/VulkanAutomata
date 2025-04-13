#include "SyncManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

VkResult createSemaphore(
    VkDevice logicalDevice,
    VkSemaphore* semaphore,
    std::vector<VkResult>* vkres) {

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    nf(&semaphoreInfo);

    VkResult result = vkCreateSemaphore(logicalDevice, &semaphoreInfo, NULL, semaphore);
    vr("vkCreateSemaphore", vkres, *semaphore, result);

    return result;
}

VkResult createFence(
    VkDevice logicalDevice,
    VkFenceCreateFlags flags,
    VkFence* fence,
    std::vector<VkResult>* vkres) {

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = flags;

    VkResult result = vkCreateFence(logicalDevice, &fenceInfo, NULL, fence);
    vr("vkCreateFence", vkres, *fence, result);

    return result;
}