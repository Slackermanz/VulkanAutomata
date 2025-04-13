#include "FramebufferManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

VkResult createFramebuffer(
    VkDevice logicalDevice,
    VkRenderPass renderPass,
    VkImageView imageView,
    uint32_t width,
    uint32_t height,
    VK_FrameBuff* framebufferData, // Output struct
    std::vector<VkResult>* vkres) {

    // 1. Setup Framebuffer Create Info
    framebufferData->fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    nf(&framebufferData->fb_info); // Set pNext to NULL and flags to 0
    framebufferData->fb_info.renderPass = renderPass;
    framebufferData->fb_info.attachmentCount = 1; // Assuming single attachment
    framebufferData->fb_info.pAttachments = &imageView;
    framebufferData->fb_info.width = width;
    framebufferData->fb_info.height = height;
    framebufferData->fb_info.layers = 1;

    // 2. Create Framebuffer
    VkResult result = vkCreateFramebuffer(logicalDevice, &framebufferData->fb_info, NULL, &framebufferData->vk_framebuffer);
    vr("vkCreateFramebuffer", vkres, framebufferData->vk_framebuffer, result);

    return result;
}