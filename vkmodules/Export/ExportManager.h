#ifndef VKMODULES_EXPORT_MANAGER_H
#define VKMODULES_EXPORT_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "../Types/Types.h"
#include "../Types/VulkanTypes.h"
#include "../Utils/Utils.h"
#include "../CellularAutomata/CellularAutomata.h" // For dft1d

// Function to handle the image export process within the main loop
void handleImageExport(
    EngineInfo* ei,             // For export flags, counters
    IMGUI_Config* gc,           // For record_imgui flag
    VK_Obj* vob,                // For logical device
    VK_QueueSync* qsync,        // For queue submission
    VK_Command* combuf_work_imagedata, // Command buffers for work->blit
    VK_Command* combuf_blit_imgui_loop, // Command buffers for swap->blit
    VK_Command* combuf_blit2buff_sing, // Command buffer for blit->buffer copy
    uint32_t frame_index,       // Current frame index
    uint32_t swap_image_index,  // Current swapchain image index
    void* pMappedBuffer,        // Mapped pointer to the export buffer (pvoid_blit2buff)
    uint32_t app_w,             // App width
    uint32_t app_h,             // App height
    GLFW_mouse glfw_mouse,      // For cursor position in save_image
    fspec256* fs,               // For DFT calculation
    fsmag256* fsm,               // For DFT calculation
    int valid,                  // *** ADDED: Pass application validity state ***
    int current_log_level,      // Current logger state to restore after export diagnostics
    std::vector<VkResult>* vkres // Result vector
);

#endif // VKMODULES_EXPORT_MANAGER_H