#include "ActionDispatcher.h"
#include "Notifications.h"
#include "../Utils/Utils.h"

void do_action(int idx, UI_info *ui, EngineInfo *ei, IMGUI_Config *gc) {

//  Toggle IMGUI
    if (idx == 0) {
        if (ei->paused) {
            if (ei->show_gui) { send_notif(6, gc); }
            else { send_notif(7, gc); }
        }
        tog(&ei->show_gui);
    }

//  Pause/Resume
    if (idx == 1) { tog(&ei->paused); }

//  Step [1] frame
    if (idx == 2) {
        ui->cmd = 0;
        ei->paused = true;
        ei->tick_loop = 1;
    }

//  Load Archive Pattern
    if (idx == 3) {
        gc->load_pattern_confirm = true;
    }

//  Random Archive Pattern
    if (idx == 4) {
        gc->load_pattern_random = true;
        gc->load_pattern_confirm = true;
    }

//  Next Archive Pattern
    if (idx == 5) {
        if(ei->PCD_count > 0) {
            gc->load_pattern_confirm = true;
            ei->load_pattern = (ei->load_pattern + 1) % ei->PCD_count;
        }
    }

//  Prev Archive Pattern
    if (idx == 6) {
        if(ei->PCD_count > 0) {
            gc->load_pattern_confirm = true;
            ei->load_pattern = (ei->load_pattern + ei->PCD_count - 1) % ei->PCD_count;
        }
    }

//  Save to PCD256 Archive
    if (idx == 7) {
        gc->save_to_archive = true;
    }

//  Fully Randomize
    if (idx == 8) {
        send_notif(11, gc);
        gc->scale_value = (float(rand() % 512 * 512) / 512.0f) + 8.0f;
        gc->zoom_value = 0.0f;
        gc->scale_update = true;
        gc->zoom_update = true;
        gc->mutate_full_random = true;
        gc->mutate_backstep_idx = gc->mutate_backstep_idx - 1;
        /*gc->mutate_backstep_idx   = -1;
        gc->mutate_backstep         = true;*/
    }

//  "Import" pattern as target
    if (idx == 9) {
        send_notif(3, gc);
        gc->mutate_backstep_idx = -1;
        gc->mutate_set_target = true;
        gc->mutate_backstep = true;
    }

//  "Update" pattern as target
//  if( idx == 19 ) {
//      send_notif(2, gc);
//      gc->mutate_backstep_idx = -1;
//      gc->mutate_backstep     = true; }

//  Mutate target pattern
    if (idx == 10) {
        //  send_notif(4, gc);
        gc->mutate_backstep_retry = true;
        gc->mutate_backstep = true;
        gc->mutate_flip = true;
    }

//  Command Shader: Random Reseed
    if (idx == 11) {
        ei->tick_loop = 1;
        ui->cmd = 1;
    }

//  Command Shader: Erase
    if (idx == 12) {
        ei->tick_loop = 1;
        ui->cmd = 2;
    }

//  Command Shader: Symmetrical Reseed
    if (idx == 13) {
        ei->tick_loop = 1;
        ui->cmd = 3;
    }

//  Shader Mode: Planar Parameter Map
    if (idx == 14) {
        if (gc->mode_planar) {
            send_notif(12, gc);
            gc->pmap_index = 0;
            gc->mode_linear = false;
            gc->mode_circular = false;
        }
    }

//  Shader Mode: Linear Parameter Map
    if (idx == 15) {
        if (gc->mode_linear) {
            send_notif(13, gc);
            gc->pmap_index = 1;
            gc->mode_planar = false;
            gc->mode_circular = false;
        }
    }

//  Shader Mode: Circular Parameter Map
    if (idx == 16) {
        if (gc->mode_circular) {
            send_notif(14, gc);
            gc->pmap_index = 2;
            gc->mode_planar = false;
            gc->mode_linear = false;
        }
    }

//  Shader Mode: Show Uniform Buffer Data
    if (idx == 17) {}

//  Toggle Frame Throttle
    if (idx == 18) {
        if (gc->throttle_enabled) { send_notif(17, gc); }
        else { send_notif(16, gc); }
        tog(&gc->throttle_enabled);
    }

//  Toggle Recording / Export to disk
    if (idx == 20) {
        if (!ei->export_enabled) { send_notif(8, gc); } else { send_notif(9, gc); }
        tog(&ei->export_enabled);
        if (ei->export_batch_left == 0
            && ei->export_batch_size > 0) {
            ei->export_batch_left = ei->export_batch_size;
        }
    }

//  Reset export image filename index
    if (idx == 21) {
        send_notif(10, gc);
        ei->imgdat_idx = 0;
    }

//  Confirm update of Scale value
    if (idx == 22) {
        gc->scale_update = true;
    }

//  Confirm update of Zoom value
    if (idx == 23) {
        gc->zoom_update = true;
    }

//  Confirm update of Backstep index
    if (idx == 24) {
        gc->mutate_backstep = true;
    }

//  Confirm update of Recording Batch Size
    if (idx == 25) {
        if(ei->export_batch_size < 0) { ei->export_batch_size = 0; }
        ei->export_batch_left = ei->export_batch_size;
        gc->mutate_backstep_last_value = gc->mutate_backstep_idx;
    }

//  Increase export frequency
    if (idx == 26) {
        ei->export_frequency++;
        send_notif_float(19, float(ei->export_frequency), gc);
    }

//  Decrease export frequency
    if (idx == 27) {
        if(ei->export_frequency > 1) { ei->export_frequency--; }
        else { ei->export_frequency = 1; }
        send_notif_float(19, float(ei->export_frequency), gc);
    }

//  Set default parameter mapping
    if (idx == 28) {
        send_notif(12, gc);
        gc->mode_planar = true;
        gc->pmap_index = 0;
    }

//  Shader Mode: Planar Parameter Map (ComboBox)
    if (idx == 29) {
        send_notif(12 + gc->pmap_index, gc);
        gc->mode_planar = (gc->pmap_index == 0) ? true : false;
        gc->mode_linear = (gc->pmap_index == 1) ? true : false;
        gc->mode_circular = (gc->pmap_index == 2) ? true : false;
    }

//  Shader Mode: Set as Planar Parameter Map
    if (idx == 30) {
        send_notif(12, gc);
        gc->pmap_index = 0;
    }

//  Shader Mode: Set as Linear Parameter Map
    if (idx == 31) {
        send_notif(13, gc);
        gc->pmap_index = 1;
    }

//  Shader Mode: Set as Circular Parameter Map
    if (idx == 32) {
        send_notif(14, gc);
        gc->pmap_index = 2;
    }

    if (idx == 33) {
        ei->tick_loop = 1;
        ui->cmd = 4;
    }

    if (idx == 34) {
        ei->tick_loop = 12;
        ui->cmd = 4;
    }

    if (idx == 35) {
        gc->zoom_value += 0.005f + gc->zoom_value * 0.05f;
        send_notif_float(18, gc->zoom_value, gc);
        gc->zoom_update = true;
    }

    if (idx == 36) {
        gc->zoom_value -= 0.005f + gc->zoom_value * 0.05f;
        send_notif_float(18, gc->zoom_value, gc);
        gc->zoom_update = true;
    }

//  Next Archive256 Pattern
    if (idx == 37) {
        if(gc->load_A256_count > 0) {
            gc->load_A256_confirm = true;
            gc->load_A256_index = (gc->load_A256_index + 1) % gc->load_A256_count;
        }
    }

//  Prev Archive256 Pattern
    if (idx == 38) {
        if(gc->load_A256_count > 0) {
            gc->load_A256_confirm = true;
            gc->load_A256_index = (gc->load_A256_index + gc->load_A256_count - 1) % gc->load_A256_count;
        }
    }

//  Rand Archive256 Pattern
    if (idx == 39) {
        if(gc->load_A256_count > 0) {
            gc->load_A256_confirm = true;
            gc->load_A256_index = rand() % gc->load_A256_count;
        }
    }

//  Load Archive256 Pattern
    if (idx == 40) {
        if(gc->load_A256_count > 0) {
            gc->load_A256_confirm = true;
        }
    }
}