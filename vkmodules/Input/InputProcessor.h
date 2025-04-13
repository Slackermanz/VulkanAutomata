#ifndef VKMODULES_INPUT_PROCESSOR_H
#define VKMODULES_INPUT_PROCESSOR_H

#include "../Types/Types.h"
#include "../UI/UI.h" // For ActionDispatcher, Notifications

// Processes mouse input not captured by ImGui
void processMouseInput(
    GLFW_mouse* mouseState,
    UI_info* ui,
    EngineInfo* ei,
    IMGUI_Config* gc,
    uint32_t app_w // Needed for scaling calculation
);

// Processes keyboard input not captured by ImGui
void processKeyboardInput(
    GLFW_key* keyState,
    UI_info* ui,
    EngineInfo* ei,
    IMGUI_Config* gc
);

#endif // VKMODULES_INPUT_PROCESSOR_H