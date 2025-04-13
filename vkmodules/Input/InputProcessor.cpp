#include "InputProcessor.h"

void processMouseInput(
    GLFW_mouse* mouseState,
    UI_info* ui,
    EngineInfo* ei,
    IMGUI_Config* gc,
    uint32_t app_w)
{
    ui->mx = mouseState->xpos;
    ui->my = mouseState->ypos;

    // MBR: Scale Panning
    if (gc->glfw_mouse_xpos_last != mouseState->xpos
        && mouseState->button == 1
        && mouseState->action != 0
        && gc->glfw_mod_LSHIFT) {
        gc->scale_has_panned = true;
        int mx_offset = mouseState->xpos - gc->glfw_mouse_xpos_last;
        float xscale = float((app_w / 2.0) - float(mx_offset)) / float(app_w);
        gc->scale_value = gc->scale_value + gc->scale_value * ((xscale - 0.5) * 2.0) * (1.0 / (1.0 + gc->zoom_value));
        gc->scale_update = true;
        send_notif_float(15, gc->scale_value, gc);
    }

    gc->glfw_mouse_xpos_last = mouseState->xpos;

    // Left Click
    if (mouseState->button == 0) { ui->mbl = mouseState->action; }

    // Right Click (handle modifier)
    if (mouseState->button == 1) { ui->mbr = mouseState->action; }
    if (mouseState->button == 1 && gc->glfw_mod_LSHIFT) { ui->mbr = 0; }

    // Middle Mouse / ScrollWheel Click
    if (mouseState->button == 2 && mouseState->action == 1) {
        mouseState->action = 0; // Consume action
        gc->mutate_backstep_last_value = gc->mutate_backstep_idx;
        do_action(9, ui, ei, gc); // Update Target
        if (gc->glfw_mod_LSHIFT) {
            gc->save_to_archive = true;
            gc->load_A256_index = -1;
        }
    }

    // Mouse Wheel Up
    if (mouseState->yoffset == -1) {
        mouseState->yoffset = 0; // Consume action
        do_action(36, ui, ei, gc); // Zoom Out
    }

    // Mouse Wheel Down
    if (mouseState->yoffset == 1) {
        mouseState->yoffset = 0; // Consume action
        do_action(35, ui, ei, gc); // Zoom In
    }

    // Mouse Back
    if (mouseState->button == 3 && mouseState->action == 1) {
        mouseState->action = 0; // Consume action
        do_action(38, ui, ei, gc); // Prev Archive256
    }

    // Mouse Forward
    if (mouseState->button == 4 && mouseState->action == 1) {
        mouseState->action = 0; // Consume action
        do_action(37, ui, ei, gc); // Next Archive256
    }
}

void processKeyboardInput(
    GLFW_key* keyState,
    UI_info* ui,
    EngineInfo* ei,
    IMGUI_Config* gc)
{
    // Release L-Shift Reseed (only if not panning)
    if (keyState->key == GLFW_KEY_LEFT_SHIFT
        && keyState->action == 0
        && !gc->scale_has_panned) {
        do_action(11, ui, ei, gc); // Reseed: Random
    }

    // Press L-Shift Sticky Modifier
    if (keyState->key == GLFW_KEY_LEFT_SHIFT && keyState->action == 1) {
        gc->glfw_mod_LSHIFT = true;
    }

    // Release L-Shift UnSticky Modifier
    if (keyState->key == GLFW_KEY_LEFT_SHIFT && keyState->action == 0) {
        gc->glfw_mod_LSHIFT = false;
        gc->scale_has_panned = false;
    }

    // Press X Clear
    if (keyState->key == GLFW_KEY_X && keyState->action >= 1) {
        do_action(12, ui, ei, gc);
    }

    // Press Z SymSeed
    if (keyState->key == GLFW_KEY_Z && keyState->action >= 1) {
        do_action(13, ui, ei, gc);
    }

    // Press C BlendSeed
    if (keyState->key == GLFW_KEY_C && keyState->action >= 1) {
        do_action(33, ui, ei, gc);
    }

    // Press TAB Show Random Archive Pattern
    if (keyState->key == GLFW_KEY_TAB && keyState->action == 1) {
        do_action(39, ui, ei, gc);
    }

    // Press RIGHT Show Next Archive Pattern
    if (keyState->key == GLFW_KEY_RIGHT && keyState->action >= 1) {
        do_action(37, ui, ei, gc);
    }

    // Press LEFT Show Prev Archive Pattern
    if (keyState->key == GLFW_KEY_LEFT && keyState->action >= 1) {
        do_action(38, ui, ei, gc);
    }

    // Press CTRL-S Save Archive Pattern
    if (keyState->key == GLFW_KEY_S
        && (keyState->mods & GLFW_MOD_CONTROL)
        && keyState->action == 1) {
        do_action(7, ui, ei, gc);
    }

    // Press T Toggle Throttle
    if (keyState->key == GLFW_KEY_T && keyState->action >= 1) {
        do_action(18, ui, ei, gc);
    }

    // Press R Full Randomization
    if (keyState->key == GLFW_KEY_R && keyState->action == 1) {
        do_action(8, ui, ei, gc);
    }

    // Press V Mutate Target
    if (keyState->key == GLFW_KEY_V && keyState->action == 1) {
        do_action(10, ui, ei, gc);
    }

    // Press Q Reload Target
    if (keyState->key == GLFW_KEY_Q && keyState->action == 1) {
        do_action(24, ui, ei, gc);
    }

    // Press KP_ENTER Toggle Recording
    if (keyState->key == GLFW_KEY_KP_ENTER && keyState->action == 1) {
        do_action(20, ui, ei, gc);
    }

    // Press KP_ADD Increase Export Freq
    if (keyState->key == GLFW_KEY_KP_ADD && keyState->action >= 1) {
        do_action(26, ui, ei, gc);
    }

    // Press KPSUBTRACT Decrease Export Freq
    if (keyState->key == GLFW_KEY_KP_SUBTRACT && keyState->action >= 1) {
        do_action(27, ui, ei, gc);
    }

    // Press 1 Planar Mapping
    if (keyState->key == GLFW_KEY_1 && keyState->action == 1) {
        do_action(30, ui, ei, gc);
        do_action(29, ui, ei, gc);
    }

    // Press 2 Linear Mapping
    if (keyState->key == GLFW_KEY_2 && keyState->action == 1) {
        do_action(31, ui, ei, gc);
        do_action(29, ui, ei, gc);
    }

    // Press 3 Circular Mapping
    if (keyState->key == GLFW_KEY_3 && keyState->action == 1) {
        do_action(32, ui, ei, gc);
        do_action(29, ui, ei, gc);
    }

    // Press S Step[1] (without CTRL)
    if (keyState->key == GLFW_KEY_S
        && !(keyState->mods & GLFW_MOD_CONTROL)
        && keyState->action >= 1) {
        do_action(2, ui, ei, gc);
    }

    // Press SPACE Toggle Pause
    if (keyState->key == GLFW_KEY_SPACE && keyState->action == 1) {
        do_action(1, ui, ei, gc);
    }

    // Press ESCAPE Toggle IMGUI
    if (keyState->key == GLFW_KEY_ESCAPE && keyState->action == 1) {
        do_action(0, ui, ei, gc);
    }
}