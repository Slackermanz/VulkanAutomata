#ifndef VKMODULES_ACTION_DISPATCHER_H
#define VKMODULES_ACTION_DISPATCHER_H

#include "../Types/Types.h"

// Process a UI action based on the action index
void do_action(int idx, UI_info *ui, EngineInfo *ei, IMGUI_Config *gc);

// Check if an input value was updated
template<typename T>
bool check_input_update(T *val, T *last) {
    if (*val != *last) { *last = *val; return true; }
    else { return false; }
}

#endif // VKMODULES_ACTION_DISPATCHER_H