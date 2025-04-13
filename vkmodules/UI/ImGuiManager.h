#ifndef VKMODULES_IMGUI_MANAGER_H
#define VKMODULES_IMGUI_MANAGER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "../../lib/imgui.h"
#include "../../lib/imgui_impl_vulkan.h"
#include "../../lib/imgui_impl_glfw.h"
#include "../Types/VulkanTypes.h" // For VK_Obj etc.
#include "../Types/Types.h"     // For EngineInfo

// Initializes Dear ImGui context, backends, descriptor pool, and font textures
VkResult initImGui(
    GLFWwindow* window,
    VK_Obj* vob,
    VkQueue graphicsQueue,
    VkRenderPass imguiRenderPass,
    uint32_t minImageCount, // From surface capabilities
    uint32_t imageCount,    // From surface capabilities or minImageCount
    EngineInfo* ei,         // To check headless mode
    std::vector<VkResult>* vkres
);

// Cleans up Dear ImGui resources
void cleanupImGui(VkDevice logicalDevice);

#endif // VKMODULES_IMGUI_MANAGER_H