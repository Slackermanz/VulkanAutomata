#ifndef VKMODULES_CONFIG_MANAGER_H
#define VKMODULES_CONFIG_MANAGER_H

#include "../Types/Types.h"
#include "../CellularAutomata/PatternManager.h" // Needed for get_PCD256_count

// Function to initialize EngineInfo struct with default values
void initEngineInfo(
    EngineInfo* ei
);

// Function to initialize IMGUI_Config struct with default values
void initImGuiConfig(
    IMGUI_Config* gc,
    EngineInfo* ei, // Needed for load_pattern
    UB32_64* pcd    // Needed for initial scale/zoom
);

#endif // VKMODULES_CONFIG_MANAGER_H