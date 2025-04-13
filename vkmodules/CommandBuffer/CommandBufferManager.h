#ifndef VKMODULES_COMMAND_BUFFER_MANAGER_H
#define VKMODULES_COMMAND_BUFFER_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../Types/VulkanTypes.h"

// Create a set of command buffers for a specific purpose
void createCommandBuffers(
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t count,
    VK_Command* commandBuffers,
    std::vector<VkResult>* vkres);

// Record work initialization commands
void recordWorkInitCommands(
    VK_Command* commandBuffer,
    VK_ImageView* workInit,
    int index,
    std::vector<VkResult>* vkres);

// Record work loop commands for simulation
void recordWorkLoopCommands(
    VK_Command* commandBuffer,
    VkRenderPassBeginInfo* renderPassInfo,
    VK_DescSetLayout3* descriptorSetLayout,
    VK_Pipe* pipeline,
    int index,
    std::vector<VkResult>* vkres);

// Record presentation commands that copy work images to swapchain
void recordPresentationCommands(
    VK_Command* commandBuffer,
    VK_Layer_2x2D* workImages,
    VkImage* swapchainImages,
    VkImageMemoryBarrier* workBarriers,
    VkImageMemoryBarrier* presentBarriers,
    VkImageBlit* imageBlit,
    uint32_t swapchainIndex,
    uint32_t workIndex,
    uint32_t swapchainCount,
    std::vector<VkResult>* vkres);

// Record image data export commands
void recordImageExportCommands(
    VK_Command* commandBuffer,
    VK_Layer_2x2D* workImages,
    VK_Layer_1x2D* blitImage,
    VkImageMemoryBarrier* workToTransferBarrier,
    VkImageMemoryBarrier* workToShaderBarrier,
    VkImageBlit* imageBlit,
    int index,
    std::vector<VkResult>* vkres);

// Record buffer to image copy commands
void recordBufferToImageCommands(
    VK_Command* commandBuffer,
    VK_Layer_1x2D* blitImage,
    VK_Buffer_1x2D* buffer,
    VkImageMemoryBarrier* imageBarrierToSrc,
    VkImageMemoryBarrier* imageBarrierToDst,
    VkBufferImageCopy* copyRegion,
    std::vector<VkResult>* vkres);

// Submit command buffer to queue and optionally wait
VkResult submitCommandBuffer(
    VkDevice device,
    VkQueue queue,
    VkSubmitInfo* submitInfo,
    VkCommandBuffer* commandBuffer,
    VkFence fence,
    bool waitForCompletion,
    std::vector<VkResult>* vkres);

#endif // VKMODULES_COMMAND_BUFFER_MANAGER_H