#define GLFW_INCLUDE_VULKAN // <<< ADD THIS DEFINE
#include <GLFW/glfw3.h>      // <<< MUST be included AFTER the define
#include "WindowManager.h"
#include "../Utils/Logger.h" // For logging helpers
#include <vector>           // Include for std::vector

// External variable defined in the main file
extern int valid; 

VkResult createGLFWWindowAndSurface(
    uint32_t width,
    uint32_t height,
    const char* title,
    VK_Obj* vob,                  // For VKI, VKP, VKQ_i
    EngineInfo* ei,             // To check headless mode
    GLFWwindow** window,          // Output: GLFW window handle
    VkSurfaceKHR* surface,        // Output: Vulkan surface handle
    VkSurfaceCapabilitiesKHR* capabilities, // Output: Surface capabilities
    std::vector<VkResult>* vkres) {

    if (ei->run_headless) {
        rv("Skipping window and surface creation in headless mode.");
        *window = nullptr;
        *surface = VK_NULL_HANDLE;
        // Optionally initialize capabilities to some default if needed downstream?
        return VK_SUCCESS;
    }

    // Window Hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create Window
    ov("glfwCreateWindow", *window);
    *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!*window) {
        ov("Error", "Failed to create GLFW window!");
        valid = 0;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Check Presentation Support
    VkResult result = (VkResult)glfwGetPhysicalDevicePresentationSupport(vob->VKI, vob->VKP, vob->VKQ_i);
    vr("glfwGetPhysicalDevicePresentationSupport", vkres, "GLFW", result);
    // Note: glfwGetPhysicalDevicePresentationSupport returns GLFW_TRUE (1) on success, not VK_SUCCESS (0)
    if (result != GLFW_TRUE) { 
        ov("Error", "Selected physical device queue does not support presentation!");
        valid = 0;
        // Don't return VK_ERROR here, let vr log the specific GLFW result code (which is likely 0, GLFW_FALSE)
        return VK_ERROR_EXTENSION_NOT_PRESENT; // Return a Vulkan error indicating lack of presentation support
    }

    // Create Vulkan Surface
    result = glfwCreateWindowSurface(vob->VKI, *window, NULL, surface);
    vr("glfwCreateWindowSurface", vkres, "GLFW", result); // glfwCreateWindowSurface returns VkResult
    if (result != VK_SUCCESS) {
        ov("Error", "Failed to create window surface!");
        valid = 0;
        return result;
    }

    // Get Surface Capabilities
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vob->VKP, *surface, capabilities);
    vr("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", vkres, "ARRAY", result);
    if (result != VK_SUCCESS) {
        ov("Error", "Failed to get surface capabilities!");
        valid = 0;
        return result;
    }
    // Log capabilities
    ov("minImageCount", capabilities->minImageCount);
    ov("maxImageCount", capabilities->maxImageCount);
    ov("currentExtent.width", capabilities->currentExtent.width);
    ov("currentExtent.height", capabilities->currentExtent.height);
    ov("maxImageArrayLayers", capabilities->maxImageArrayLayers);
    ov("supportedCompositeAlpha", capabilities->supportedCompositeAlpha);
    ov("supportedUsageFlags", capabilities->supportedUsageFlags);

    // Get Surface Formats (Optional but good practice)
    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(vob->VKP, *surface, &formatCount, NULL);
    vr("vkGetPhysicalDeviceSurfaceFormatsKHR (count)", vkres, formatCount, result);
    if (result == VK_SUCCESS && formatCount > 0) {
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(vob->VKP, *surface, &formatCount, formats.data());
        vr("vkGetPhysicalDeviceSurfaceFormatsKHR (formats)", vkres, "ARRAY", result);
        for (uint32_t i = 0; i < formatCount; ++i) {
            iv("vk_surface_format.format", formats[i].format, i);
            iv("vk_surface_format.colorSpace", formats[i].colorSpace, i);
        }
    }

    // Check Surface Support
    VkBool32 surfaceSupported = VK_FALSE;
    result = vkGetPhysicalDeviceSurfaceSupportKHR(vob->VKP, vob->VKQ_i, *surface, &surfaceSupported);
    vr("vkGetPhysicalDeviceSurfaceSupportKHR", vkres, vob->VKQ_i, result);
    ov("Surface Supported", ((surfaceSupported == VK_TRUE) ? "TRUE" : "FALSE"));
    if (result != VK_SUCCESS || surfaceSupported != VK_TRUE) {
        ov("Error", "Selected queue does not support the created surface!");
        valid = 0;
        return result == VK_SUCCESS ? VK_ERROR_SURFACE_LOST_KHR : result;
    }

    return VK_SUCCESS;
}

void cleanupGLFW(GLFWwindow* window) {
    if (window) {
        rv("glfwDestroyWindow");
        glfwDestroyWindow(window);
    }
    // Note: glfwTerminate() is usually called once at the very end of the application
    // It might be better placed outside this module, perhaps in main() after all cleanup.
    // rv("glfwTerminate");
    // glfwTerminate();
}