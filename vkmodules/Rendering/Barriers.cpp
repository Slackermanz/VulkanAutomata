#include "Barriers.h"

void initializeBarriers(
    VK_Obj* vob,
    VK_Layer_1x2D* blit,
    VK_Layer_2x2D* work,
    VK_RPConfig* rpass_info,
    const std::vector<VkImage>& swapChainImages,
    uint32_t swap_image_count,
    // Output Barrier Structs
    VkImageMemoryBarrier* vk_IMB_blit_TSO_to_TDO,
    VkImageMemoryBarrier* vk_IMB_blit_TDO_to_TSO,
    VkImageMemoryBarrier* vk_IMB_blit_imagedata_UND_to_TDO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_CAO_to_PRS,
    std::vector<VkImageMemoryBarrier>& vk_IMB_work_SRO_to_TSO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_work_TSO_to_SRO,
    VkImageMemoryBarrier* vk_IMB_blit_imagedata_TDO_to_TSO,
    VkImageMemoryBarrier* vk_IMB_blit_imagedata_TSO_to_TDO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_UND_to_PRS,
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_PRS_to_TDO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_TDO_to_PRS,
    std::vector<VkImageMemoryBarrier>& vk_IMB_swap_PRS_to_TSO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_swap_TSO_to_PRS
) {

    // Resize vectors
    vk_IMB_pres_CAO_to_PRS.resize(swap_image_count);
    vk_IMB_pres_UND_to_PRS.resize(swap_image_count);
    vk_IMB_pres_PRS_to_TDO.resize(swap_image_count);
    vk_IMB_pres_TDO_to_PRS.resize(swap_image_count);
    vk_IMB_swap_PRS_to_TSO.resize(swap_image_count);
    vk_IMB_swap_TSO_to_PRS.resize(swap_image_count);
    vk_IMB_work_SRO_to_TSO.resize(2);
    vk_IMB_work_TSO_to_SRO.resize(2);

    // --- Initialize single barriers ---
    vk_IMB_blit_TSO_to_TDO->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vk_IMB_blit_TSO_to_TDO->pNext = NULL;
    vk_IMB_blit_TSO_to_TDO->srcAccessMask = 0;
    vk_IMB_blit_TSO_to_TDO->dstAccessMask = 0;
    vk_IMB_blit_TSO_to_TDO->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk_IMB_blit_TSO_to_TDO->newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk_IMB_blit_TSO_to_TDO->srcQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_TSO_to_TDO->dstQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_TSO_to_TDO->image = blit->vk_image;
    vk_IMB_blit_TSO_to_TDO->subresourceRange = rpass_info->img_subres_range;

    vk_IMB_blit_TDO_to_TSO->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vk_IMB_blit_TDO_to_TSO->pNext = NULL;
    vk_IMB_blit_TDO_to_TSO->srcAccessMask = 0;
    vk_IMB_blit_TDO_to_TSO->dstAccessMask = 0;
    vk_IMB_blit_TDO_to_TSO->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk_IMB_blit_TDO_to_TSO->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk_IMB_blit_TDO_to_TSO->srcQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_TDO_to_TSO->dstQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_TDO_to_TSO->image = blit->vk_image;
    vk_IMB_blit_TDO_to_TSO->subresourceRange = rpass_info->img_subres_range;

    vk_IMB_blit_imagedata_UND_to_TDO->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vk_IMB_blit_imagedata_UND_to_TDO->pNext = NULL;
    vk_IMB_blit_imagedata_UND_to_TDO->srcAccessMask = 0;
    vk_IMB_blit_imagedata_UND_to_TDO->dstAccessMask = 0;
    vk_IMB_blit_imagedata_UND_to_TDO->oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vk_IMB_blit_imagedata_UND_to_TDO->newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk_IMB_blit_imagedata_UND_to_TDO->srcQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_imagedata_UND_to_TDO->dstQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_imagedata_UND_to_TDO->image = blit->vk_image;
    vk_IMB_blit_imagedata_UND_to_TDO->subresourceRange = rpass_info->img_subres_range;

    vk_IMB_blit_imagedata_TDO_to_TSO->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vk_IMB_blit_imagedata_TDO_to_TSO->pNext = NULL;
    vk_IMB_blit_imagedata_TDO_to_TSO->srcAccessMask = 0;
    vk_IMB_blit_imagedata_TDO_to_TSO->dstAccessMask = 0;
    vk_IMB_blit_imagedata_TDO_to_TSO->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk_IMB_blit_imagedata_TDO_to_TSO->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk_IMB_blit_imagedata_TDO_to_TSO->srcQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_imagedata_TDO_to_TSO->dstQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_imagedata_TDO_to_TSO->image = blit->vk_image;
    vk_IMB_blit_imagedata_TDO_to_TSO->subresourceRange = rpass_info->img_subres_range;

    vk_IMB_blit_imagedata_TSO_to_TDO->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vk_IMB_blit_imagedata_TSO_to_TDO->pNext = NULL;
    vk_IMB_blit_imagedata_TSO_to_TDO->srcAccessMask = 0;
    vk_IMB_blit_imagedata_TSO_to_TDO->dstAccessMask = 0;
    vk_IMB_blit_imagedata_TSO_to_TDO->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk_IMB_blit_imagedata_TSO_to_TDO->newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vk_IMB_blit_imagedata_TSO_to_TDO->srcQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_imagedata_TSO_to_TDO->dstQueueFamilyIndex = vob->VKQ_i;
    vk_IMB_blit_imagedata_TSO_to_TDO->image = blit->vk_image;
    vk_IMB_blit_imagedata_TSO_to_TDO->subresourceRange = rpass_info->img_subres_range;

    // --- Initialize barrier arrays/vectors ---
    for (uint32_t i = 0; i < swap_image_count; ++i) {
        vk_IMB_pres_CAO_to_PRS[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_pres_CAO_to_PRS[i].pNext = NULL;
        vk_IMB_pres_CAO_to_PRS[i].srcAccessMask = 0;
        vk_IMB_pres_CAO_to_PRS[i].dstAccessMask = 0;
        vk_IMB_pres_CAO_to_PRS[i].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vk_IMB_pres_CAO_to_PRS[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vk_IMB_pres_CAO_to_PRS[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_CAO_to_PRS[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_CAO_to_PRS[i].image = swapChainImages[i];
        vk_IMB_pres_CAO_to_PRS[i].subresourceRange = rpass_info->img_subres_range;

        vk_IMB_pres_UND_to_PRS[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_pres_UND_to_PRS[i].pNext = NULL;
        vk_IMB_pres_UND_to_PRS[i].srcAccessMask = 0;
        vk_IMB_pres_UND_to_PRS[i].dstAccessMask = 0;
        vk_IMB_pres_UND_to_PRS[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vk_IMB_pres_UND_to_PRS[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vk_IMB_pres_UND_to_PRS[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_UND_to_PRS[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_UND_to_PRS[i].image = swapChainImages[i];
        vk_IMB_pres_UND_to_PRS[i].subresourceRange = rpass_info->img_subres_range;

        vk_IMB_pres_PRS_to_TDO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_pres_PRS_to_TDO[i].pNext = NULL;
        vk_IMB_pres_PRS_to_TDO[i].srcAccessMask = 0;
        vk_IMB_pres_PRS_to_TDO[i].dstAccessMask = 0;
        vk_IMB_pres_PRS_to_TDO[i].oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vk_IMB_pres_PRS_to_TDO[i].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vk_IMB_pres_PRS_to_TDO[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_PRS_to_TDO[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_PRS_to_TDO[i].image = swapChainImages[i];
        vk_IMB_pres_PRS_to_TDO[i].subresourceRange = rpass_info->img_subres_range;

        vk_IMB_pres_TDO_to_PRS[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_pres_TDO_to_PRS[i].pNext = NULL;
        vk_IMB_pres_TDO_to_PRS[i].srcAccessMask = 0;
        vk_IMB_pres_TDO_to_PRS[i].dstAccessMask = 0;
        vk_IMB_pres_TDO_to_PRS[i].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vk_IMB_pres_TDO_to_PRS[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vk_IMB_pres_TDO_to_PRS[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_TDO_to_PRS[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_pres_TDO_to_PRS[i].image = swapChainImages[i];
        vk_IMB_pres_TDO_to_PRS[i].subresourceRange = rpass_info->img_subres_range;

        vk_IMB_swap_PRS_to_TSO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_swap_PRS_to_TSO[i].pNext = NULL;
        vk_IMB_swap_PRS_to_TSO[i].srcAccessMask = 0;
        vk_IMB_swap_PRS_to_TSO[i].dstAccessMask = 0;
        vk_IMB_swap_PRS_to_TSO[i].oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vk_IMB_swap_PRS_to_TSO[i].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vk_IMB_swap_PRS_to_TSO[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_swap_PRS_to_TSO[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_swap_PRS_to_TSO[i].image = swapChainImages[i];
        vk_IMB_swap_PRS_to_TSO[i].subresourceRange = rpass_info->img_subres_range;

        vk_IMB_swap_TSO_to_PRS[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_swap_TSO_to_PRS[i].pNext = NULL;
        vk_IMB_swap_TSO_to_PRS[i].srcAccessMask = 0;
        vk_IMB_swap_TSO_to_PRS[i].dstAccessMask = 0;
        vk_IMB_swap_TSO_to_PRS[i].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vk_IMB_swap_TSO_to_PRS[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        vk_IMB_swap_TSO_to_PRS[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_swap_TSO_to_PRS[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_swap_TSO_to_PRS[i].image = swapChainImages[i];
        vk_IMB_swap_TSO_to_PRS[i].subresourceRange = rpass_info->img_subres_range;
    }

    for (int i = 0; i < 2; ++i) {
        vk_IMB_work_SRO_to_TSO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_work_SRO_to_TSO[i].pNext = NULL;
        vk_IMB_work_SRO_to_TSO[i].srcAccessMask = 0;
        vk_IMB_work_SRO_to_TSO[i].dstAccessMask = 0;
        vk_IMB_work_SRO_to_TSO[i].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk_IMB_work_SRO_to_TSO[i].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vk_IMB_work_SRO_to_TSO[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_work_SRO_to_TSO[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_work_SRO_to_TSO[i].image = work->vk_image[i];
        vk_IMB_work_SRO_to_TSO[i].subresourceRange = rpass_info->img_subres_range;

        vk_IMB_work_TSO_to_SRO[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_IMB_work_TSO_to_SRO[i].pNext = NULL;
        vk_IMB_work_TSO_to_SRO[i].srcAccessMask = 0;
        vk_IMB_work_TSO_to_SRO[i].dstAccessMask = 0;
        vk_IMB_work_TSO_to_SRO[i].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        vk_IMB_work_TSO_to_SRO[i].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk_IMB_work_TSO_to_SRO[i].srcQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_work_TSO_to_SRO[i].dstQueueFamilyIndex = vob->VKQ_i;
        vk_IMB_work_TSO_to_SRO[i].image = work->vk_image[i];
        vk_IMB_work_TSO_to_SRO[i].subresourceRange = rpass_info->img_subres_range;
    }
}