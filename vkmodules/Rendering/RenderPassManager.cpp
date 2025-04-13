#include "RenderPassManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

// --- Existing function createSimpleColorRenderPass remains here ---

VkResult createSimpleColorRenderPass(
    VkDevice logicalDevice,
    VkFormat format,
    VkAttachmentLoadOp loadOp,
    VkAttachmentStoreOp storeOp,
    VkImageLayout initialLayout,
    VkImageLayout finalLayout,
    VK_RenderPass* renderPassData, // Output struct
    std::vector<VkResult>* vkres) {

    // 1. Attachment Description (Single Color Attachment)
    renderPassData->attach_desc.flags = 0;
    renderPassData->attach_desc.format = format;
    renderPassData->attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    renderPassData->attach_desc.loadOp = loadOp;
    renderPassData->attach_desc.storeOp = storeOp;
    renderPassData->attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    renderPassData->attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    renderPassData->attach_desc.initialLayout = initialLayout;
    renderPassData->attach_desc.finalLayout = finalLayout;

    // 2. Attachment Reference
    renderPassData->attach_ref.attachment = 0; // Index of the attachment in the pAttachments array
    renderPassData->attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // 3. Subpass Description
    renderPassData->subpass_desc.flags = 0;
    renderPassData->subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    renderPassData->subpass_desc.inputAttachmentCount = 0;
    renderPassData->subpass_desc.pInputAttachments = NULL;
    renderPassData->subpass_desc.colorAttachmentCount = 1; // One color attachment
    renderPassData->subpass_desc.pColorAttachments = &renderPassData->attach_ref;
    renderPassData->subpass_desc.pResolveAttachments = NULL;
    renderPassData->subpass_desc.pDepthStencilAttachment = NULL;
    renderPassData->subpass_desc.preserveAttachmentCount = 0;
    renderPassData->subpass_desc.pPreserveAttachments = NULL;

    // 4. Render Pass Create Info
    renderPassData->rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    nf(&renderPassData->rp_info); // Set pNext to NULL and flags to 0
    renderPassData->rp_info.attachmentCount = 1; // One attachment
    renderPassData->rp_info.pAttachments = &renderPassData->attach_desc;
    renderPassData->rp_info.subpassCount = 1; // One subpass
    renderPassData->rp_info.pSubpasses = &renderPassData->subpass_desc;
    renderPassData->rp_info.dependencyCount = 0; // No dependencies for this simple case
    renderPassData->rp_info.pDependencies = NULL;

    // 5. Create Render Pass
    VkResult result = vkCreateRenderPass(logicalDevice, &renderPassData->rp_info, NULL, &renderPassData->vk_render_pass);
    vr("vkCreateRenderPass", vkres, renderPassData->vk_render_pass, result);

    return result;
}

// --- NEW FUNCTION ADDED BELOW ---

void setupRenderPassConfiguration(
    uint32_t width,
    uint32_t height,
    VK_RPConfig* rpConfig // Output struct
) {
    // Rect2D (Render Area / Scissor)
    rpConfig->rect2D.offset.x = 0;
    rpConfig->rect2D.offset.y = 0;
    rpConfig->rect2D.extent.width = width;
    rpConfig->rect2D.extent.height = height;

    // Clear Value
    rpConfig->clear_val.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Default clear color

    // Image Subresource Range (Commonly used for barriers/views)
    rpConfig->img_subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    rpConfig->img_subres_range.baseMipLevel = 0;
    rpConfig->img_subres_range.levelCount = 1;
    rpConfig->img_subres_range.baseArrayLayer = 0;
    rpConfig->img_subres_range.layerCount = 1;

    // Image Subresource Layers (Commonly used for blits/copies)
    rpConfig->img_subres_layer.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    rpConfig->img_subres_layer.mipLevel = 0;
    rpConfig->img_subres_layer.baseArrayLayer = 0;
    rpConfig->img_subres_layer.layerCount = 1;

    // Image Blit region
    rpConfig->img_blit.srcSubresource = rpConfig->img_subres_layer;
    rpConfig->img_blit.srcOffsets[0] = {0, 0, 0};
    rpConfig->img_blit.srcOffsets[1] = {(int32_t)width, (int32_t)height, 1};
    rpConfig->img_blit.dstSubresource = rpConfig->img_subres_layer;
    rpConfig->img_blit.dstOffsets[0] = {0, 0, 0};
    rpConfig->img_blit.dstOffsets[1] = {(int32_t)width, (int32_t)height, 1};

    // Buffer-Image Copy region
    rpConfig->buffer_img_cpy.bufferOffset = 0;
    rpConfig->buffer_img_cpy.bufferRowLength = width;
    rpConfig->buffer_img_cpy.bufferImageHeight = height;
    rpConfig->buffer_img_cpy.imageSubresource = rpConfig->img_subres_layer;
    rpConfig->buffer_img_cpy.imageOffset = {0, 0, 0};
    rpConfig->buffer_img_cpy.imageExtent = {width, height, 1};

    // Viewport
    rpConfig->vk_viewport.x = 0.0f;
    rpConfig->vk_viewport.y = 0.0f;
    rpConfig->vk_viewport.width = (float)width;
    rpConfig->vk_viewport.height = (float)height;
    rpConfig->vk_viewport.minDepth = 0.0f;
    rpConfig->vk_viewport.maxDepth = 1.0f;

    // Note: Sampler creation is handled separately (SamplerManager)
    // rpConfig->vk_sampler is initialized elsewhere.
}