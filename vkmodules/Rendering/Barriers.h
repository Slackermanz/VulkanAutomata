#ifndef VKMODULES_BARRIERS_H
#define VKMODULES_BARRIERS_H

#include <vulkan/vulkan.h>
#include "../Types/VulkanTypes.h" // For VK_Layer_1x2D, VK_Layer_2x2D
#include "../Types/Types.h" // For swap_image_count potentially (though better to pass)
#include "../Resources/Resources.h" // For VK_RPConfig
#include "../VulkanFoundation/VulkanFoundation.h" // For VK_Obj

// Function to initialize all barriers (needs access to many setup structs)
// Note: This approach requires passing many handles. Consider if defining
// them in main() was actually clearer, despite the line count.
void initializeBarriers(
    VK_Obj* vob,
    VK_Layer_1x2D* blit,
    VK_Layer_2x2D* work,
    VK_RPConfig* rpass_info,
    const std::vector<VkImage>& swapChainImages, // Use const ref
    uint32_t swap_image_count,
    // Output Barrier Structs (passed by pointer)
    VkImageMemoryBarrier* vk_IMB_blit_TSO_to_TDO,
    VkImageMemoryBarrier* vk_IMB_blit_TDO_to_TSO,
    VkImageMemoryBarrier* vk_IMB_blit_imagedata_UND_to_TDO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_CAO_to_PRS, // Use vector ref
    std::vector<VkImageMemoryBarrier>& vk_IMB_work_SRO_to_TSO, // Use vector ref
    std::vector<VkImageMemoryBarrier>& vk_IMB_work_TSO_to_SRO, // Use vector ref
    VkImageMemoryBarrier* vk_IMB_blit_imagedata_TDO_to_TSO,
    VkImageMemoryBarrier* vk_IMB_blit_imagedata_TSO_to_TDO,
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_UND_to_PRS, // Use vector ref
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_PRS_to_TDO, // Use vector ref
    std::vector<VkImageMemoryBarrier>& vk_IMB_pres_TDO_to_PRS, // Use vector ref
    std::vector<VkImageMemoryBarrier>& vk_IMB_swap_PRS_to_TSO, // Use vector ref
    std::vector<VkImageMemoryBarrier>& vk_IMB_swap_TSO_to_PRS  // Use vector ref
);

#endif // VKMODULES_BARRIERS_H