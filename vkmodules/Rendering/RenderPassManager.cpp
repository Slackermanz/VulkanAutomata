#include "RenderPassManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

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