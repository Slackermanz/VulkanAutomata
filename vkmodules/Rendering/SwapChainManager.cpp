#include "SwapChainManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf, ov)
#include <limits> // Required for std::numeric_limits
#include <algorithm> // Required for std::clamp

VkResult createSwapChain(
    VK_Obj* vob,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* capabilities,
    uint32_t queueFamilyIndex,
    VkSwapchainKHR* oldSwapchain,
    VkSwapchainKHR* swapChain,
    uint32_t* imageCount, // In/Out parameter
    std::vector<VkImage>* swapChainImages,
    std::vector<VkResult>* vkres) {

    // Choose Swapchain format (B8G8R8A8 UNORM and SRGB Non-linear are common)
    VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    // Choose presentation mode (Immediate is used in original code)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

    // Choose swap extent
    VkExtent2D extent = capabilities->currentExtent;

    // Determine image count
    uint32_t desiredImageCount = capabilities->minImageCount + 1;
    if (capabilities->maxImageCount > 0 && desiredImageCount > capabilities->maxImageCount) {
        desiredImageCount = capabilities->maxImageCount;
    }
    *imageCount = desiredImageCount;
    ov("Swapchain desiredImageCount", *imageCount);

    // Create Swapchain Info struct
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    nf(&createInfo);
    createInfo.surface = surface;
    createInfo.minImageCount = *imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    // *** CORRECTED USAGE FLAGS ***
    // Added VK_IMAGE_USAGE_TRANSFER_SRC_BIT as required by the blit operation
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &queueFamilyIndex;
    createInfo.preTransform = capabilities->currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // Set to true as per common practice
    createInfo.oldSwapchain = *oldSwapchain;

    // Create Swapchain
    VkResult result = vkCreateSwapchainKHR(vob->VKL, &createInfo, NULL, swapChain);
    vr("vkCreateSwapchainKHR", vkres, *swapChain, result);
    if (result != VK_SUCCESS) {
        return result;
    }

    // Retrieve Swapchain Images
    result = vkGetSwapchainImagesKHR(vob->VKL, *swapChain, imageCount, NULL);
    vr("vkGetSwapchainImagesKHR (count)", vkres, *imageCount, result);
    if (result != VK_SUCCESS) {
        // Cleanup created swapchain?
        return result;
    }

    swapChainImages->resize(*imageCount);
    result = vkGetSwapchainImagesKHR(vob->VKL, *swapChain, imageCount, swapChainImages->data());
    vr("vkGetSwapchainImagesKHR (handles)", vkres, "ARRAY", result);

    for(uint32_t i = 0; i < *imageCount; ++i) {
        iv("Swapchain Image", (*swapChainImages)[i], i);
    }

    return result;
}