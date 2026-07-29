#ifndef VKMODULES_VULKAN_TYPES_H
#define VKMODULES_VULKAN_TYPES_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <fstream>

// Shader management - moved from Types.h
struct ShaderCodeInfo {
    std::string         shaderFilename;
    std::vector<char>   shaderData;
    size_t              shaderBytes;
    bool                shaderBytesValid;
};

// Function to load shader code from file
inline ShaderCodeInfo getShaderCodeInfo(const std::string& filename) {
    ShaderCodeInfo sc_info;
        sc_info.shaderFilename      = filename;
        sc_info.shaderData          = std::vector<char>();
        sc_info.shaderBytes         = 0;
        sc_info.shaderBytesValid    = false;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if(!file.is_open()) { return sc_info; }

    std::streampos endPos = file.tellg();
    if(endPos == std::streampos(-1) || endPos <= std::streampos(0)) { return sc_info; }

    size_t fileSize = static_cast<size_t>(endPos);
    if(fileSize % 4 != 0) { return sc_info; }

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    if(file.gcount() != static_cast<std::streamsize>(fileSize)) { return sc_info; }
    file.close();

        sc_info.shaderData          = buffer;
        sc_info.shaderBytes         = buffer.size();
        sc_info.shaderBytesValid    = true;
    return sc_info;
}

// Core Vulkan objects
struct VK_Obj {
    VkInstance          VKI;
    uint32_t            VKP_i;
    VkPhysicalDevice    VKP;
    uint32_t            VKQ_i;
    VkDevice            VKL;
};

struct VK_Config {
    VkApplicationInfo       app_info;
    VkInstanceCreateInfo    inst_info;
};

struct VK_Debug {
    VkDebugUtilsMessengerCreateInfoEXT  debug_msg_info;
    VkDebugUtilsMessengerEXT            vk_debug_utils_messenger_ext;
};

struct VK_PhysDev {
    uint32_t                        pd_count;
    VkPhysicalDevice                vk_pdev;
    VkPhysicalDeviceProperties      vk_pdev_props;
    VkPhysicalDeviceFeatures        vk_pdev_feats;
    VkPhysicalDeviceMemoryProperties    vk_pdev_mem_props;
};

struct VK_PDQueues {
    VkDeviceQueueCreateInfo     pdq_info; 
};

struct VK_LogDev {
    VkDeviceCreateInfo          ldev_info;
};

struct VK_Layer_1x2D {
    VkExtent3D              ext3D;
    VkImageCreateInfo       img_info;
    VkImage                 vk_image;
    uint32_t                MTB_index;
    VkMemoryRequirements    vk_mem_reqs;
    VkMemoryAllocateInfo    vk_mem_allo_info;
    VkDeviceMemory          vk_dev_mem;
};

struct VK_Buffer_1x2D {
    VkBufferCreateInfo      buff_info;
    VkBuffer                vk_buffer;
    uint32_t                MTB_index;
    VkMemoryRequirements    vk_mem_reqs;
    VkMemoryAllocateInfo    vk_mem_allo_info;
    VkDeviceMemory          vk_dev_mem;
};

struct VK_Buffer_Data {
    VkBufferCreateInfo      buff_info;
    VkBuffer                vk_buffer;
    uint32_t                MTB_index;
    VkMemoryRequirements    vk_mem_reqs;
    VkMemoryAllocateInfo    vk_mem_allo_info;
    VkDeviceMemory          vk_dev_mem;
};

struct VK_Layer_2x2D {
    VkExtent3D              ext3D[2];
    VkImageCreateInfo       img_info[2];
    VkImage                 vk_image[2];
    uint32_t                MTB_index[2];
    VkMemoryRequirements    vk_mem_reqs[2];
    VkMemoryAllocateInfo    vk_mem_allo_info[2];
    VkDeviceMemory          vk_dev_mem[2];
};

struct VK_Command {
    VkCommandPoolCreateInfo         pool_info;
    VkCommandPool                   vk_command_pool;
    VkCommandBufferAllocateInfo     comm_buff_alloc_info;
    VkCommandBuffer                 vk_command_buffer;
    VkCommandBufferBeginInfo        comm_buff_begin_info;
};

struct ShaderData {
    ShaderCodeInfo              SC_info;
    VkShaderModuleCreateInfo    vk_SM_info;
    VkShaderModule              vk_shader_module;
    VkShaderStageFlagBits       stage_bits;
};

struct VK_RPConfig {
    VkRect2D                            rect2D;
    VkClearValue                        clear_val;
    VkImageSubresourceRange             img_subres_range;
    VkImageSubresourceLayers            img_subres_layer;
    VkImageBlit                     img_blit;
    VkBufferImageCopy                   buffer_img_cpy;
    VkViewport                      vk_viewport;
    VkSamplerCreateInfo                 samp_info;
    VkSampler                       vk_sampler;
};

struct VK_PipeInfo {
    VkPipelineRasterizationStateCreateInfo      p_rast_info;
    VkPipelineViewportStateCreateInfo           p_vprt_info;
    VkPipelineMultisampleStateCreateInfo        p_msam_info;
    VkPipelineVertexInputStateCreateInfo        p_vtin_info;
    VkPipelineInputAssemblyStateCreateInfo      p_inas_info;
    VkPipelineColorBlendAttachmentState         p_cbat_info;
    VkPipelineColorBlendStateCreateInfo         p_cbst_info;
    VkPipelineShaderStageCreateInfo         p_shad_info[2]; // VERT_FLS+FRAG_FLS
};

struct VK_ImageView {
    VkImageViewCreateInfo   img_view_info;
    VkImageView         vk_image_view;
    VkImageMemoryBarrier    img_mem_barr;
};

struct VK_QueueSync {
    VkQueue             vk_queue;
    VkSubmitInfo        sub_info;
    VkFenceCreateInfo   fence_info;
    VkFence             vk_fence;
};

struct VK_DescSetLayout {
    VkDescriptorSetLayoutBinding        set_bind[2];
    VkDescriptorSetLayoutCreateInfo     set_info;
    VkDescriptorPoolSize                pool_size[2];
    VkDescriptorPoolCreateInfo          pool_info;
    VkDescriptorSetLayout               vk_desc_set_layout;
    VkDescriptorPool                    vk_desc_pool;
    VkDescriptorSetAllocateInfo         allo_info;
    VkDescriptorSet                 vk_descriptor_set[2];
};

struct VK_DescSetLayout3 {
    VkDescriptorSetLayoutBinding        set_bind[3];
    VkDescriptorSetLayoutCreateInfo     set_info;
    VkDescriptorPoolSize                pool_size[3];
    VkDescriptorPoolCreateInfo          pool_info;
    VkDescriptorSetLayout               vk_desc_set_layout;
    VkDescriptorPool                    vk_desc_pool;
    VkDescriptorSetAllocateInfo         allo_info;
    VkDescriptorSet                 vk_descriptor_set[3];
};

struct VK_RenderPass {
    VkAttachmentDescription     attach_desc;
    VkAttachmentReference       attach_ref;
    VkSubpassDescription        subpass_desc;
    VkRenderPassCreateInfo      rp_info;
    VkRenderPass                vk_render_pass;
};

struct VK_FrameBuff {
    VkFramebufferCreateInfo     fb_info;
    VkFramebuffer               vk_framebuffer;
};

struct VK_Pipe {
    VkPipelineLayoutCreateInfo      layout_info;
    VkPipelineLayout                vk_pipeline_layout;
    VkGraphicsPipelineCreateInfo    gfx_pipe_info;
    VkPipeline                  vk_pipeline;
};

#endif // VKMODULES_VULKAN_TYPES_H