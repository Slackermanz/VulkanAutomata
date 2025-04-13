#include "SyncManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

// --- Existing createSemaphore and createFence functions remain here ---

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

// --- Existing getDeviceQueue and setupBasicSubmitInfo functions remain here ---

void getDeviceQueue(
    VkDevice logicalDevice,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    VkQueue* queue) {

    rv("vkGetDeviceQueue");
    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, queueIndex, queue);
}

void setupBasicSubmitInfo(
    VK_QueueSync* qSync) {

    qSync->sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    qSync->sub_info.pNext = NULL;
    qSync->sub_info.waitSemaphoreCount = 0;
    qSync->sub_info.pWaitSemaphores = NULL;
    qSync->sub_info.pWaitDstStageMask = NULL;
    qSync->sub_info.commandBufferCount = 1; // Default to 1, caller sets pCommandBuffers
    qSync->sub_info.pCommandBuffers = NULL;
    qSync->sub_info.signalSemaphoreCount = 0;
    qSync->sub_info.pSignalSemaphores = NULL;
}

// --- CORRECTED FUNCTIONS BELOW ---

void setupPresentSubmitInfo(
    VkSemaphore* pWaitSemaphore, // *** CHANGED: Pointer ***
    VkPipelineStageFlags* pWaitStageMask, // *** CHANGED: Pointer ***
    VkSemaphore* pSignalSemaphore, // *** CHANGED: Pointer ***
    VK_QueueSync* qSync)
{
    qSync->sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    qSync->sub_info.pNext = NULL;
    qSync->sub_info.waitSemaphoreCount = 1;
    qSync->sub_info.pWaitSemaphores = pWaitSemaphore; // *** Assign pointer directly ***
    qSync->sub_info.pWaitDstStageMask = pWaitStageMask; // *** Assign pointer directly ***
    qSync->sub_info.commandBufferCount = 1; // Default to 1, caller sets pCommandBuffers
    qSync->sub_info.pCommandBuffers = NULL;
    qSync->sub_info.signalSemaphoreCount = 1;
    qSync->sub_info.pSignalSemaphores = pSignalSemaphore; // *** Assign pointer directly ***
}

void setupImGuiSubmitInfo(
    VkSemaphore* pWaitSemaphore, // *** CHANGED: Pointer ***
    VkPipelineStageFlags* pWaitStageMask, // *** CHANGED: Pointer ***
    VkSemaphore* pSignalSemaphore, // *** CHANGED: Pointer ***
    VK_QueueSync* qSync)
{
    qSync->sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    qSync->sub_info.pNext = NULL;
    qSync->sub_info.waitSemaphoreCount = 1;
    qSync->sub_info.pWaitSemaphores = pWaitSemaphore; // *** Assign pointer directly ***
    qSync->sub_info.pWaitDstStageMask = pWaitStageMask; // *** Assign pointer directly ***
    qSync->sub_info.commandBufferCount = 1; // Default to 1, caller sets pCommandBuffers
    qSync->sub_info.pCommandBuffers = NULL;
    qSync->sub_info.signalSemaphoreCount = 1;
    qSync->sub_info.pSignalSemaphores = pSignalSemaphore; // *** Assign pointer directly ***
}