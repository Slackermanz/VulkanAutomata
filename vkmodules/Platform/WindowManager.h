#ifndef VKMODULES_WINDOW_MANAGER_H
#define VKMODULES_WINDOW_MANAGER_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "../Types/VulkanTypes.h" // For VK_Obj
#include "../Types/Types.h"     // For EngineInfo

// Function to create GLFW window and Vulkan surface
VkResult createGLFWWindowAndSurface(
    uint32_t width,
    uint32_t height,
    const char* title,
    VK_Obj* vob,                  // For VKI, VKP, VKQ_i
    EngineInfo* ei,             // To check headless mode
    GLFWwindow** window,          // Output: GLFW window handle
    VkSurfaceKHR* surface,        // Output: Vulkan surface handle
    VkSurfaceCapabilitiesKHR* capabilities, // Output: Surface capabilities
    std::vector<VkResult>* vkres
);

// Function to destroy the window and terminate GLFW
void cleanupGLFW(GLFWwindow* window);

#endif // VKMODULES_WINDOW_MANAGER_H