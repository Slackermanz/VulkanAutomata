#include "SamplerManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

VkResult createDefaultSampler(
    VkDevice logicalDevice,
    VkSampler* sampler, // Output sampler handle
    std::vector<VkResult>* vkres) {

    VkSamplerCreateInfo samp_info = {};
    samp_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    nf(&samp_info);
    samp_info.magFilter = VK_FILTER_NEAREST;
    samp_info.minFilter = VK_FILTER_NEAREST;
    samp_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samp_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samp_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samp_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samp_info.mipLodBias = 1.0f;
    samp_info.anisotropyEnable = VK_FALSE;
    samp_info.maxAnisotropy = 1.0f;
    samp_info.compareEnable = VK_FALSE;
    samp_info.compareOp = VK_COMPARE_OP_NEVER;
    samp_info.minLod = 1.0f;
    samp_info.maxLod = 1.0f;
    samp_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samp_info.unnormalizedCoordinates = VK_FALSE;

    VkResult result = vkCreateSampler(logicalDevice, &samp_info, NULL, sampler);
    vr("vkCreateSampler", vkres, *sampler, result);

    return result;
}