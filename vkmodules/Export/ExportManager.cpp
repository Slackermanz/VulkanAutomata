#include "ExportManager.h"

// External variable defined in the main file (for verbose_loops only now)
extern uint32_t verbose_loops;

// Local definition of MAXLOG for this module
const int LOCAL_MAXLOG = 2; // Or get from config if needed

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
    int valid,                  // *** ADDED parameter ***
    int current_log_level,      // Current logger state to restore after export diagnostics
    std::vector<VkResult>* vkres // Result vector
) {

    // Check if export is enabled and conditions are met
    if ((!ei->paused || ei->tick_loop)
        && valid // *** Use passed-in valid ***
        && ei->export_enabled
        && ei->export_frequency > 0
        && frame_index % ei->export_frequency == 0
        && frame_index > 0) {

        // Determine which image to copy from (work or swapchain+ImGui)
        bool get_gui = false;
        if (ei->run_headless || !gc->record_imgui || !ei->show_gui) {
            rv("combuf_work_imagedata");
            qsync->sub_info.pCommandBuffers = &combuf_work_imagedata[(frame_index + 0) % 2].vk_command_buffer;
        } else {
            get_gui = true;
            rv("combuf_blit_imgui_loop");
            qsync->sub_info.pCommandBuffers = &combuf_blit_imgui_loop[swap_image_index].vk_command_buffer;
        }

        // Submit the first copy (work->blit or swap->blit)
        vr("vkQueueSubmit (Export Copy 1)", vkres, qsync->sub_info.pCommandBuffers,
           vkQueueSubmit(qsync->vk_queue, 1, &qsync->sub_info, VK_NULL_HANDLE));
        vr("vkDeviceWaitIdle (Export Copy 1)", vkres, "IDLE",
           vkDeviceWaitIdle(vob->VKL));

        // Submit the second copy (blit->buffer)
        rv("combuf_blit2buff_sing");
        qsync->sub_info.pCommandBuffers = &combuf_blit2buff_sing[0].vk_command_buffer;
        vr("vkQueueSubmit (Export Copy 2)", vkres, qsync->sub_info.pCommandBuffers,
           vkQueueSubmit(qsync->vk_queue, 1, &qsync->sub_info, VK_NULL_HANDLE));
        vr("vkDeviceWaitIdle (Export Copy 2)", vkres, "IDLE",
           vkDeviceWaitIdle(vob->VKL));

        // Save the image from the mapped buffer
        NS_Timer optime = start_timer(optime); // Assuming NS_Timer and start_timer are available via includes
        int originalLogLevel = current_log_level; // Store original log level
        if (!verbose_loops) { loglevel = LOCAL_MAXLOG; } // *** Use LOCAL_MAXLOG ***
        save_image(pMappedBuffer, "IMG" + std::to_string(ei->imgdat_idx), app_w, app_h, glfw_mouse, get_gui);
        end_timer(optime, "Save ImageData"); // Assuming end_timer is available
        if (!verbose_loops) { loglevel = originalLogLevel; } // Restore original log level

        // Update export counters and perform DFT
        ei->imgdat_idx++;
        dft1d(ei->imgdat_idx * 735, 512, fs, fsm);

        // Handle batch export pausing
        if (ei->export_batch_size > 0
            && ei->export_batch_left > 0) {
            ei->export_batch_left--;
        }
        if (!ei->run_headless
            && ei->export_batch_size > 0
            && ei->export_batch_left == 0) {
            ei->paused = true;
            ei->export_batch_left = ei->export_batch_size;
        }
    }
}