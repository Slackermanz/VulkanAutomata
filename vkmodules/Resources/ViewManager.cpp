#include "ViewManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

VkResult createImageView(
    VkDevice logicalDevice,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    VK_ImageView* imageViewData, // Output struct
    std::vector<VkResult>* vkres) {

    // 1. Setup Image View Create Info
    imageViewData->img_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    nf(&imageViewData->img_view_info); // Set pNext to NULL and flags to 0
    imageViewData->img_view_info.image = image;
    imageViewData->img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewData->img_view_info.format = format;
    // Set component swizzles to identity (default)
    imageViewData->img_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewData->img_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewData->img_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewData->img_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    // Setup subresource range
    imageViewData->img_view_info.subresourceRange.aspectMask = aspectFlags;
    imageViewData->img_view_info.subresourceRange.baseMipLevel = 0;
    imageViewData->img_view_info.subresourceRange.levelCount = 1;
    imageViewData->img_view_info.subresourceRange.baseArrayLayer = 0;
    imageViewData->img_view_info.subresourceRange.layerCount = 1;

    // 2. Create Image View
    VkResult result = vkCreateImageView(logicalDevice, &imageViewData->img_view_info, NULL, &imageViewData->vk_image_view);
    vr("vkCreateImageView", vkres, imageViewData->vk_image_view, result);

    // Note: We are not setting up the ImageMemoryBarrier here,
    // as that's usually context-specific and done before vkCmdPipelineBarrier.
    // The VK_ImageView struct still has the img_mem_barr field, but it's uninitialized.

    return result;
}