#include "ConfigManager.h"
#include <cstring> // For memcpy

void initEngineInfo(EngineInfo* ei) {
    ei->paused             = false;
    ei->show_gui           = true;
    ei->run_headless       = false;
    ei->imgdat_idx         = 0;
    ei->export_frequency   = 8;
    ei->export_batch_size  = 180;
    ei->export_batch_left  = ei->export_batch_size;
    ei->export_batch_last  = ei->export_batch_size;
    ei->export_enabled     = false;
    ei->tick_loop          = 0;
    ei->load_pattern       = 18372;
    ei->PCD_count          = 0; // This will be updated later if needed
}

void initImGuiConfig(IMGUI_Config* gc, EngineInfo* ei, UB32_64* pcd) {
    gc->load_shader                  = false;
    gc->load_pattern                 = false;
    gc->load_pattern_confirm         = false;
    gc->load_pattern_check_instant   = true;
    gc->load_pattern_check_reseed    = true;
    gc->load_pattern_random          = false;
    gc->save_to_archive              = false;
    gc->mutate_menu                  = false;
    gc->mutate_full_random           = false;
    gc->mutate_backstep              = false;
    gc->mutate_backstep_retry        = false;
    gc->mutate_flip                  = false;
    gc->throttle_menu                = false;
    gc->throttle_enabled             = false;
    gc->mode_planar                  = true;
    gc->mode_linear                  = false;
    gc->mode_circular                = false;
    gc->mode_showdata                = false;
    gc->scale_zoom_menu              = false;
    gc->scale_update                 = false;
    gc->zoom_update                  = false;
    gc->glfw_mod_LCTRL               = false;
    gc->glfw_mod_LSHIFT              = false;
    gc->show_notification_float      = false;
    gc->scale_has_panned             = false;
    gc->recording_config             = false;
    gc->record_imgui                 = true;
    gc->load_A256_confirm            = true; // Start with confirm true to load initial pattern

    gc->load_pattern_last_value      = ei->load_pattern;
    gc->mutate_flip_str              = 80;
    gc->mutate_backstep_idx          = -1;
    gc->mutate_backstep_last_value   = gc->mutate_backstep_idx;
    gc->throttle_target              = 32;
    gc->pmap_index                   = 0;
    gc->pmap_index_last              = gc->pmap_index;
    gc->glfw_mouse_xpos_last         = 0; // Initialize to 0, will be updated
    gc->load_A256_index              = -1;
    gc->load_A256_index_last         = gc->load_A256_index;
    gc->load_A256_count              = get_PCD256_count("sav/PCD256_archive.vkpat");

    gc->notification_float_value     = 0.0f;

    // Initialize scale/zoom from pcd (assuming pcd is initialized first)
    memcpy(&gc->scale_value, &pcd->u32[62], sizeof(uint32_t));
    gc->scale_last_value             = gc->scale_value;
    memcpy(&gc->zoom_value, &pcd->u32[61], sizeof(uint32_t));
    gc->zoom_last_value              = gc->zoom_value;

    // Initialize notification timer fields outside this function
    // gc->notification_timer = ...;
    // gc->notification_float_timer = ...;
    // gc->show_notification = ...;
    // gc->notification_age = ...;
    // gc->notification_float_age = ...;
}