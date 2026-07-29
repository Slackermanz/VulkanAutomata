#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanCore.h"
#include "../Utils/Logger.h"
#include <iostream>
#include <vector>
#include <limits> // Needed for UINT32_MAX
#include <cstring>

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
    if (glfwInit() != GLFW_TRUE) {
        ov("Error", "glfwInit failed.");
        valid = 0;
        return false;
    }
    rv("glfwInit");

    if (glfwVulkanSupported() != GLFW_TRUE) {
        ov("Error", "glfwVulkanSupported returned false.");
        valid = 0;
        return false;
    }
    rv("glfwVulkanSupported");
    
    rv("glfwGetRequiredInstanceExtensions");
    *glfw_extensions = glfwGetRequiredInstanceExtensions(glfw_ext_count);

    if (*glfw_extensions == NULL || *glfw_ext_count == 0) {
        ov("Error", "glfwGetRequiredInstanceExtensions returned no Vulkan surface extensions.");
        valid = 0;
        return false;
    }
    
    for(int i = 0; i < *glfw_ext_count; i++) {
        iv("GLFW Extensions", (*glfw_extensions)[i], i);
    }
    
    return true;
}

bool isInstanceExtensionAvailable(const char* extension_name, std::vector<VkResult>* vkres) {
    uint32_t extension_count = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    vr("vkEnumerateInstanceExtensionProperties (count)", vkres, extension_count, result);
    if (result != VK_SUCCESS) {
        return false;
    }

    std::vector<VkExtensionProperties> extensions(extension_count);
    result = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.data());
    vr("vkEnumerateInstanceExtensionProperties (list)", vkres, "ARRAY", result);
    if (result != VK_SUCCESS) {
        return false;
    }

    for (const auto& extension : extensions) {
        if (std::strcmp(extension.extensionName, extension_name) == 0) {
            return true;
        }
    }

    return false;
}

bool isInstanceLayerAvailable(const char* layer_name, std::vector<VkResult>* vkres) {
    uint32_t layer_count = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    vr("vkEnumerateInstanceLayerProperties (count)", vkres, layer_count, result);
    if (result != VK_SUCCESS) {
        return false;
    }

    std::vector<VkLayerProperties> layers(layer_count);
    result = vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
    vr("vkEnumerateInstanceLayerProperties (list)", vkres, "ARRAY", result);
    if (result != VK_SUCCESS) {
        return false;
    }

    for (const auto& layer : layers) {
        if (std::strcmp(layer.layerName, layer_name) == 0) {
            return true;
        }
    }

    return false;
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
    
    if (PFN_VKCDUM == nullptr) {
        ov("Error", "Failed to load vkCreateDebugUtilsMessengerEXT! Extension may not be available.");
        valid = 0;
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    
    // Create debug messenger
    VkResult result = PFN_VKCDUM(vob->VKI, &vkdbg->debug_msg_info, NULL, &vkdbg->vk_debug_utils_messenger_ext);
    vr("vkCreateDebugUtilsMessengerEXT", vkres, vkdbg->vk_debug_utils_messenger_ext, result);
    
    return result;
}

// --- NEW FUNCTIONS ADDED BELOW ---

VkResult selectPhysicalDevice(
    VkInstance instance,
    VK_Obj* vob, // Output: Stores selected device handle and index
    VK_PhysDev* selectedPdevInfo, // Output: Stores properties/features of selected device
    std::vector<VkResult>* vkres) {

    uint32_t PDev_Count = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &PDev_Count, NULL);
    vr("vkEnumeratePhysicalDevices (count)", vkres, PDev_Count, result);
    if (result != VK_SUCCESS || PDev_Count == 0) {
        ov("Error", "Failed to find GPUs with Vulkan support!");
        valid = 0;
        return result == VK_SUCCESS ? VK_ERROR_INITIALIZATION_FAILED : result;
    }

    std::vector<VkPhysicalDevice> devices(PDev_Count);
    result = vkEnumeratePhysicalDevices(instance, &PDev_Count, devices.data());
    vr("vkEnumeratePhysicalDevices (handles)", vkres, "ARRAY", result);
    if (result != VK_SUCCESS) {
        valid = 0;
        return result;
    }

    std::vector<VK_PhysDev> pdevInfos(PDev_Count);
    uint32_t PDev_Index = UINT32_MAX;

    // Get properties and features for all devices
    for (uint32_t i = 0; i < PDev_Count; ++i) {
        pdevInfos[i].pd_count = PDev_Count;
        pdevInfos[i].vk_pdev = devices[i];
        rv("vkGetPhysicalDeviceProperties");
        vkGetPhysicalDeviceProperties(pdevInfos[i].vk_pdev, &pdevInfos[i].vk_pdev_props);
        rv("vkGetPhysicalDeviceFeatures");
        vkGetPhysicalDeviceFeatures(pdevInfos[i].vk_pdev, &pdevInfos[i].vk_pdev_feats);

        // Log properties (optional, can be reduced based on loglevel)
        if (loglevel >= 1) {
            iv("Physical Devices", pdevInfos[i].vk_pdev, i);
            iv("deviceName", pdevInfos[i].vk_pdev_props.deviceName, i);
            iv("deviceType", pdevInfos[i].vk_pdev_props.deviceType, i);
            // Add more property logging if needed
        }
    }

    // Select device based on type preference
    uint32_t PDev_TypeList[5] = { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 
                                  VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, 
                                  VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, 
                                  VK_PHYSICAL_DEVICE_TYPE_CPU, 
                                  VK_PHYSICAL_DEVICE_TYPE_OTHER };

    for (int j = 0; j < 5; ++j) {
        if (PDev_Index == UINT32_MAX) {
            for (uint32_t i = 0; i < PDev_Count; ++i) {
                if (PDev_Index == UINT32_MAX && pdevInfos[i].vk_pdev_props.deviceType == PDev_TypeList[j]) {
                    PDev_Index = i;
                    ov("Selected PDev_Index", i);
                    ov("Selected PDev_Index deviceType", PDev_TypeList[j]);
                    break; // Found preferred type, stop inner loop
                }
            }
        }
        if (PDev_Index != UINT32_MAX) break; // Found a device, stop outer loop
        ov("No devices found of type", PDev_TypeList[j]);
    }

    if (PDev_Index == UINT32_MAX) {
        ov("Error", "Failed to find a suitable GPU!");
        valid = 0;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Store selected device info
    vob->VKP_i = PDev_Index;
    vob->VKP = pdevInfos[PDev_Index].vk_pdev;
    *selectedPdevInfo = pdevInfos[PDev_Index]; // Copy properties/features

    // Log memory properties of selected device
    rv("vkGetPhysicalDeviceMemoryProperties");
    vkGetPhysicalDeviceMemoryProperties(vob->VKP, &selectedPdevInfo->vk_pdev_mem_props);

    if (loglevel >= 1) {
        ov("Selected Device memoryTypeCount", selectedPdevInfo->vk_pdev_mem_props.memoryTypeCount);
        // Add more memory logging if needed
    }

    return VK_SUCCESS;
}

VkResult findGraphicsQueueFamily(
    VkPhysicalDevice physicalDevice,
    uint32_t* queueFamilyIndex, // Output
    uint32_t* queueCount,      // Output
    std::vector<VkResult>* vkres) {

    uint32_t PDev_QFP_Count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &PDev_QFP_Count, NULL);
    ov("PDev Queue Family Props Count", PDev_QFP_Count);

    if (PDev_QFP_Count == 0) {
        ov("Error", "Physical device has no queue families!");
        valid = 0;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::vector<VkQueueFamilyProperties> queueProps(PDev_QFP_Count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &PDev_QFP_Count, queueProps.data());

    uint32_t PDev_QFP_Index = UINT32_MAX;
    for (uint32_t i = 0; i < PDev_QFP_Count; ++i) {
        if (loglevel >= 1) {
            iv("Queue Family queueFlags", queueProps[i].queueFlags, i);
            iv("Queue Family queueCount", queueProps[i].queueCount, i);
        }
        if (PDev_QFP_Index == UINT32_MAX && (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            PDev_QFP_Index = i;
            *queueFamilyIndex = i;
            *queueCount = queueProps[i].queueCount;
            ov("Selected PDev_QFP_Index", i);
            ov("Selected PDev_QFP_Index queueFlags", queueProps[i].queueFlags);
            ov("Selected PDev_QFP_Index queueCount", queueProps[i].queueCount);
            // Don't break, keep logging all queues if loglevel is high
        }
    }

    if (PDev_QFP_Index == UINT32_MAX) {
        ov("Error", "Could not find a queue family supporting VK_QUEUE_GRAPHICS_BIT!");
        valid = 0;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

void setupDeviceQueueCreateInfo(
    uint32_t queueFamilyIndex,
    uint32_t queueCount,
    const float* pQueuePriorities,
    VK_PDQueues* pdq) {

    pdq->pdq_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    nf(&pdq->pdq_info);
    pdq->pdq_info.queueFamilyIndex = queueFamilyIndex;
    pdq->pdq_info.queueCount = queueCount; // Use the actual count from the family
    pdq->pdq_info.pQueuePriorities = pQueuePriorities;
}

// --- NEW FUNCTION ADDED BELOW ---

VkResult createLogicalDevice(
    VkPhysicalDevice physicalDevice,
    VK_PDQueues* pdq,               // Contains queue create info
    const char** device_extensions, // Device extensions to enable
    uint32_t device_extension_count,
    VkPhysicalDeviceFeatures* enabledFeatures, // Features to enable
    VK_Obj* vob,                    // Output: Stores logical device handle (VKL)
    std::vector<VkResult>* vkres) {

    VK_LogDev ldev; // Local struct to hold create info

    ldev.ldev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    nf(&ldev.ldev_info);
    ldev.ldev_info.queueCreateInfoCount = 1;
    ldev.ldev_info.pQueueCreateInfos = &pdq->pdq_info;
    ldev.ldev_info.enabledLayerCount = 0;
    ldev.ldev_info.ppEnabledLayerNames = NULL;
    ldev.ldev_info.enabledExtensionCount = device_extension_count;
    ldev.ldev_info.ppEnabledExtensionNames = device_extensions;
    ldev.ldev_info.pEnabledFeatures = enabledFeatures;

    VkResult result = vkCreateDevice(physicalDevice, &ldev.ldev_info, NULL, &vob->VKL);
    vr("vkCreateDevice", vkres, vob->VKL, result);
    ov("VkDevice", vob->VKL);

    return result;
}