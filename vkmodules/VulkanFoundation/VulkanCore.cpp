#include "VulkanCore.h"
#include "../Utils/Logger.h"
#include <iostream>
#include <vector>

// External variables
extern int loglevel;
extern int valid;

// Local definition of MAXLOG for this module - same value as in main file
const int LOCAL_MAXLOG = 2;

VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                       VkDebugUtilsMessageTypeFlagsEXT messageType, 
                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                 void* pUserData) {
    std::string bar = "";
    for(int i = 0; i < 20; i++) { bar = bar + "####"; }

    std::string msg     = pCallbackData->pMessage;
    std::string msg_fmt = "";
    for(int i = 0; i < msg.size()-1; i++) {
        char chr = msg[i];
        char chrhtml = msg[i+1];
        msg_fmt = ( 
            chr == ':' && chrhtml != '/' ?
                msg_fmt+":\n\n" : ( chr == '|' ?
                    msg_fmt+"\n" : msg_fmt+chr ) ); }

    msg_fmt = msg_fmt + msg[msg.size()-1];
    if(messageSeverity != 16) {
        loglevel = LOCAL_MAXLOG;
        if(loglevel >= 0) { std::cout << "\n\n" << bar << "\n " << msg_fmt << "\n" << bar << "\n\n"; }
        std::cout << "  Validation messageSeverity: " << messageSeverity << "\n\n";
        valid = 0; }
    return VK_FALSE;
}

int32_t findProperties(const VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                      uint32_t memoryTypeBitsRequirement,
                      VkMemoryPropertyFlags requiredProperties) {
    const uint32_t memoryCount = pMemoryProperties->memoryTypeCount;
    for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) {
        const uint32_t memoryTypeBits = (1 << memoryIndex);
        const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;
        const VkMemoryPropertyFlags properties = pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
        const bool hasRequiredProperties = (properties & requiredProperties) == requiredProperties;
        if (isRequiredMemoryType && hasRequiredProperties) { 
            return static_cast<int32_t>(memoryIndex);
        }
    }
    return -1;
}

bool initGLFWExtensions(uint32_t* glfw_ext_count, const char*** glfw_extensions, std::vector<VkResult>* vkres) {
    vr("glfwInit", vkres, "GLFW", VkResult(glfwInit()));
    vr("glfwVulkanSupported", vkres, "GLFW", VkResult(glfwVulkanSupported()));
    
    rv("glfwGetRequiredInstanceExtensions");
    *glfw_extensions = glfwGetRequiredInstanceExtensions(glfw_ext_count);
    
    for(int i = 0; i < *glfw_ext_count; i++) {
        iv("GLFW Extensions", (*glfw_extensions)[i], i);
    }
    
    return true;
}

VkResult initVulkanInstance(VK_Obj* vob, VK_Config* vkcfg, 
                         const char** instance_extensions, uint32_t instance_extension_count,
                         const char** layer_extensions, uint32_t layer_extension_count,
                         std::vector<VkResult>* vkres) {
    vr("init", vkres, "INIT", VK_ERROR_UNKNOWN);
    
    // Setup application info
    vkcfg->app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkcfg->app_info.pNext = NULL;
    vkcfg->app_info.pApplicationName = "VulkanAutomataGLFW";
    vkcfg->app_info.applicationVersion = 0;
    vkcfg->app_info.pEngineName = NULL;
    vkcfg->app_info.engineVersion = 0;
    vkcfg->app_info.apiVersion = VK_API_VERSION_1_2;
    
    // Setup instance create info
    vkcfg->inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    nf(&vkcfg->inst_info);
    vkcfg->inst_info.pApplicationInfo = &vkcfg->app_info;
    vkcfg->inst_info.enabledLayerCount = layer_extension_count;
    vkcfg->inst_info.ppEnabledLayerNames = layer_extensions;
    vkcfg->inst_info.enabledExtensionCount = instance_extension_count;
    vkcfg->inst_info.ppEnabledExtensionNames = instance_extensions;
    
    // Create Vulkan instance
    VkResult result = vkCreateInstance(&vkcfg->inst_info, NULL, &vob->VKI);
    vr("vkCreateInstance", vkres, vob->VKI, result);
    
    return result;
}

VkResult setupDebugMessenger(VK_Obj* vob, VK_Debug* vkdbg, std::vector<VkResult>* vkres) {
    // Setup debug messenger create info
    vkdbg->debug_msg_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    nf(&vkdbg->debug_msg_info);
    vkdbg->debug_msg_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT 
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    vkdbg->debug_msg_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    vkdbg->debug_msg_info.pfnUserCallback = debugCallback;
    
    // Get function pointer for debug utils creation
    rv("vkGetInstanceProcAddr");
    auto PFN_VKCDUM = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(vob->VKI, "vkCreateDebugUtilsMessengerEXT");
    
    // Create debug messenger
    VkResult result = PFN_VKCDUM(vob->VKI, &vkdbg->debug_msg_info, NULL, &vkdbg->vk_debug_utils_messenger_ext);
    vr("vkCreateDebugUtilsMessengerEXT", vkres, vkdbg->vk_debug_utils_messenger_ext, result);
    
    return result;
}