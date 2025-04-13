#include "ImGuiManager.h"
#include "../Utils/Logger.h"
#include <stdexcept>
#include <vector>

// Need to store the descriptor pool globally within this module for cleanup
namespace {
    VkDescriptorPool g_imguiDescriptorPool = VK_NULL_HANDLE;
    VK_Command g_imguiFontUploadCmdBuf;
}

VkResult initImGui(
    GLFWwindow* window,
    VK_Obj* vob,
    VkQueue graphicsQueue,
    VkRenderPass imguiRenderPass,
    uint32_t minImageCount,
    uint32_t imageCount,
    EngineInfo* ei,
    std::vector<VkResult>* vkres) {

    if (ei->run_headless) {
        rv("Skipping ImGui initialization in headless mode.");
        return VK_SUCCESS;
    }

    ov("IMGUI Version", IMGUI_CHECKVERSION());

    rv("ImGui::CreateContext");
    ImGui::CreateContext();

    rv("ImGui::StyleColorsDark");
    ImGui::StyleColorsDark();

    rv("ImGui::GetIO");
    ImGuiIO &io = ImGui::GetIO(); // Can configure io flags here if needed

    rv("ImGui_ImplGlfw_InitForVulkan");
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Create Descriptor Pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    nf(&pool_info);
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkResult result = vkCreateDescriptorPool(vob->VKL, &pool_info, nullptr, &g_imguiDescriptorPool);
    vr("vkCreateDescriptorPool (ImGui)", vkres, g_imguiDescriptorPool, result);
    if (result != VK_SUCCESS) {
        return result; // Failed to create descriptor pool
    }

    // Initialize ImGui Vulkan Implementation
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vob->VKI;
    init_info.PhysicalDevice = vob->VKP;
    init_info.Device = vob->VKL;
    init_info.QueueFamily = vob->VKQ_i;
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = g_imguiDescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = minImageCount;
    init_info.ImageCount = imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr; // Can add a check function if needed

    rv("ImGui_ImplVulkan_Init");
    ImGui_ImplVulkan_Init(&init_info, imguiRenderPass);

    // Upload Fonts
    // Use a temporary command buffer for font creation
    g_imguiFontUploadCmdBuf.pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    nf(&g_imguiFontUploadCmdBuf.pool_info);
    g_imguiFontUploadCmdBuf.pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    g_imguiFontUploadCmdBuf.pool_info.queueFamilyIndex = vob->VKQ_i;

    result = vkCreateCommandPool(vob->VKL, &g_imguiFontUploadCmdBuf.pool_info, nullptr, &g_imguiFontUploadCmdBuf.vk_command_pool);
    vr("vkCreateCommandPool (ImGui Fonts)", vkres, g_imguiFontUploadCmdBuf.vk_command_pool, result);
    if (result != VK_SUCCESS) return result;

    g_imguiFontUploadCmdBuf.comm_buff_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    g_imguiFontUploadCmdBuf.comm_buff_alloc_info.pNext = NULL;
    g_imguiFontUploadCmdBuf.comm_buff_alloc_info.commandPool = g_imguiFontUploadCmdBuf.vk_command_pool;
    g_imguiFontUploadCmdBuf.comm_buff_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    g_imguiFontUploadCmdBuf.comm_buff_alloc_info.commandBufferCount = 1;

    result = vkAllocateCommandBuffers(vob->VKL, &g_imguiFontUploadCmdBuf.comm_buff_alloc_info, &g_imguiFontUploadCmdBuf.vk_command_buffer);
    vr("vkAllocateCommandBuffers (ImGui Fonts)", vkres, g_imguiFontUploadCmdBuf.vk_command_buffer, result);
    if (result != VK_SUCCESS) return result;

    g_imguiFontUploadCmdBuf.comm_buff_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    nf(&g_imguiFontUploadCmdBuf.comm_buff_begin_info);
    g_imguiFontUploadCmdBuf.comm_buff_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(g_imguiFontUploadCmdBuf.vk_command_buffer, &g_imguiFontUploadCmdBuf.comm_buff_begin_info);
    vr("vkBeginCommandBuffer (ImGui Fonts)", vkres, g_imguiFontUploadCmdBuf.vk_command_buffer, result);
    if (result != VK_SUCCESS) return result;

    rv("ImGui_ImplVulkan_CreateFontsTexture");
    ImGui_ImplVulkan_CreateFontsTexture(g_imguiFontUploadCmdBuf.vk_command_buffer);

    result = vkEndCommandBuffer(g_imguiFontUploadCmdBuf.vk_command_buffer);
    vr("vkEndCommandBuffer (ImGui Fonts)", vkres, g_imguiFontUploadCmdBuf.vk_command_buffer, result);
    if (result != VK_SUCCESS) return result;

    // Submit the command buffer
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &g_imguiFontUploadCmdBuf.vk_command_buffer;

    // Need a fence to wait for completion
    VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence fence;
    result = vkCreateFence(vob->VKL, &fenceInfo, nullptr, &fence);
    if (result != VK_SUCCESS) return result;

    result = vkQueueSubmit(graphicsQueue, 1, &submit_info, fence);
    vr("vkQueueSubmit (ImGui Fonts)", vkres, graphicsQueue, result);
    if (result != VK_SUCCESS) {
        vkDestroyFence(vob->VKL, fence, nullptr);
        return result;
    }

    result = vkWaitForFences(vob->VKL, 1, &fence, VK_TRUE, UINT64_MAX);
    vr("vkWaitForFences (ImGui Fonts)", vkres, fence, result);

    // Cleanup temporary fence and command buffer/pool
    vkDestroyFence(vob->VKL, fence, nullptr);
    vkFreeCommandBuffers(vob->VKL, g_imguiFontUploadCmdBuf.vk_command_pool, 1, &g_imguiFontUploadCmdBuf.vk_command_buffer);
    vkDestroyCommandPool(vob->VKL, g_imguiFontUploadCmdBuf.vk_command_pool, nullptr);
    g_imguiFontUploadCmdBuf.vk_command_pool = VK_NULL_HANDLE; // Mark as destroyed
    g_imguiFontUploadCmdBuf.vk_command_buffer = VK_NULL_HANDLE;

    rv("ImGui_ImplVulkan_DestroyFontUploadObjects");
    ImGui_ImplVulkan_DestroyFontUploadObjects(); // Destroy font texture staging resources

    return result; // Return result of vkWaitForFences or earlier error
}

void cleanupImGui(VkDevice logicalDevice) {
    if (g_imguiDescriptorPool != VK_NULL_HANDLE) {
        rv("vkDestroyDescriptorPool (ImGui)");
        vkDestroyDescriptorPool(logicalDevice, g_imguiDescriptorPool, nullptr);
        g_imguiDescriptorPool = VK_NULL_HANDLE;
    }
    if (g_imguiFontUploadCmdBuf.vk_command_pool != VK_NULL_HANDLE) {
        // Should have been destroyed after font upload, but just in case
        rv("vkDestroyCommandPool (ImGui Fonts - cleanup)");
        vkDestroyCommandPool(logicalDevice, g_imguiFontUploadCmdBuf.vk_command_pool, nullptr);
    }
    rv("ImGui_ImplVulkan_Shutdown");
    ImGui_ImplVulkan_Shutdown();
    rv("ImGui_ImplGlfw_Shutdown");
    ImGui_ImplGlfw_Shutdown();
    rv("ImGui::DestroyContext");
    ImGui::DestroyContext();
}
