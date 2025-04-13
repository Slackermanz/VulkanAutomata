#include "DescriptorManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf, ov)

VkResult createWorkDescriptorSet(
    VkDevice logicalDevice,
    VK_DescSetLayout3* descSetLayoutData, // Input/Output struct
    uint32_t setCount, // Usually 2
    std::vector<VkResult>* vkres) {

    // 1. Define Bindings (Hardcoded for the specific Work pipeline layout)
    // Binding 0: Uniform Buffer (UBO)
    descSetLayoutData->set_bind[0].binding = 0;
    descSetLayoutData->set_bind[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descSetLayoutData->set_bind[0].descriptorCount = 1;
    descSetLayoutData->set_bind[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descSetLayoutData->set_bind[0].pImmutableSamplers = NULL;
    // Binding 1: Combined Image Sampler
    descSetLayoutData->set_bind[1].binding = 1;
    descSetLayoutData->set_bind[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descSetLayoutData->set_bind[1].descriptorCount = 1;
    descSetLayoutData->set_bind[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descSetLayoutData->set_bind[1].pImmutableSamplers = NULL;
    // Binding 2: Storage Buffer (SSBO)
    descSetLayoutData->set_bind[2].binding = 2;
    descSetLayoutData->set_bind[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descSetLayoutData->set_bind[2].descriptorCount = 1;
    descSetLayoutData->set_bind[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descSetLayoutData->set_bind[2].pImmutableSamplers = NULL;

    // 2. Create Descriptor Set Layout Info
    descSetLayoutData->set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    nf(&descSetLayoutData->set_info);
    descSetLayoutData->set_info.bindingCount = 3; // UBO, Sampler, SSBO
    descSetLayoutData->set_info.pBindings = descSetLayoutData->set_bind;

    // 3. Create Descriptor Set Layout
    VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &descSetLayoutData->set_info, NULL, &descSetLayoutData->vk_desc_set_layout);
    vr("vkCreateDescriptorSetLayout", vkres, descSetLayoutData->vk_desc_set_layout, result);
    if (result != VK_SUCCESS) return result;

    // 4. Define Pool Sizes (ensure enough for the number of sets)
    descSetLayoutData->pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descSetLayoutData->pool_size[0].descriptorCount = setCount;
    descSetLayoutData->pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descSetLayoutData->pool_size[1].descriptorCount = setCount;
    descSetLayoutData->pool_size[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descSetLayoutData->pool_size[2].descriptorCount = setCount;

    // 5. Create Descriptor Pool Info
    descSetLayoutData->pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    nf(&descSetLayoutData->pool_info);
    descSetLayoutData->pool_info.maxSets = setCount;
    descSetLayoutData->pool_info.poolSizeCount = 3;
    descSetLayoutData->pool_info.pPoolSizes = descSetLayoutData->pool_size;

    ov("Descriptor pool_info.maxSets", descSetLayoutData->pool_info.maxSets);

    // 6. Create Descriptor Pool
    result = vkCreateDescriptorPool(logicalDevice, &descSetLayoutData->pool_info, NULL, &descSetLayoutData->vk_desc_pool);
    vr("vkCreateDescriptorPool", vkres, descSetLayoutData->vk_desc_pool, result);
    if (result != VK_SUCCESS) {
        vkDestroyDescriptorSetLayout(logicalDevice, descSetLayoutData->vk_desc_set_layout, NULL);
        return result;
    }

    // 7. Allocate Descriptor Sets Info
    // Need to create a temporary layout array for allocation
    std::vector<VkDescriptorSetLayout> layouts(setCount, descSetLayoutData->vk_desc_set_layout);
    descSetLayoutData->allo_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descSetLayoutData->allo_info.pNext = NULL;
    descSetLayoutData->allo_info.descriptorPool = descSetLayoutData->vk_desc_pool;
    descSetLayoutData->allo_info.descriptorSetCount = setCount;
    descSetLayoutData->allo_info.pSetLayouts = layouts.data();

    // 8. Allocate Descriptor Sets (Store in the first `setCount` elements of vk_descriptor_set)
    // Ensure vk_descriptor_set array in VK_DescSetLayout3 is large enough (it is, size 3)
    result = vkAllocateDescriptorSets(logicalDevice, &descSetLayoutData->allo_info, descSetLayoutData->vk_descriptor_set);
    vr("vkAllocateDescriptorSets", vkres, descSetLayoutData->vk_descriptor_set[0], result); // Log first handle
    if (result != VK_SUCCESS) {
        vkDestroyDescriptorPool(logicalDevice, descSetLayoutData->vk_desc_pool, NULL);
        vkDestroyDescriptorSetLayout(logicalDevice, descSetLayoutData->vk_desc_set_layout, NULL);
        return result;
    }

    return VK_SUCCESS;
}

void updateWorkDescriptorSets(
    VkDevice logicalDevice,
    VK_DescSetLayout3* descSetLayoutData, // Contains allocated sets
    VkSampler sampler,
    VK_ImageView work_init[2], // *** CORRECTED: Accept array of VK_ImageView structs ***
    VkBuffer uniformBuffer,
    VkBuffer storageBuffer,
    uint32_t setCount) { // Usually 2

    // Prepare Descriptor Buffer/Image Infos
    VkDescriptorImageInfo imageInfo[2];
    imageInfo[0].sampler = sampler;
    imageInfo[0].imageView = work_init[1].vk_image_view; // *** CORRECTED: Access handle from struct ***
    imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo[1].sampler = sampler;
    imageInfo[1].imageView = work_init[0].vk_image_view; // *** CORRECTED: Access handle from struct ***
    imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorBufferInfo uboInfo;
    uboInfo.buffer = uniformBuffer;
    uboInfo.offset = 0;
    uboInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo ssboInfo;
    ssboInfo.buffer = storageBuffer;
    ssboInfo.offset = 0;
    ssboInfo.range = VK_WHOLE_SIZE;

    // Prepare Write Descriptor Set structures (3 writes per set)
    std::vector<VkWriteDescriptorSet> writeSets(setCount * 3);

    for (uint32_t i = 0; i < setCount; ++i) {
        // Write for UBO (Binding 0)
        writeSets[i * 3 + 0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSets[i * 3 + 0].pNext = NULL;
        writeSets[i * 3 + 0].dstSet = descSetLayoutData->vk_descriptor_set[i];
        writeSets[i * 3 + 0].dstBinding = 0;
        writeSets[i * 3 + 0].dstArrayElement = 0;
        writeSets[i * 3 + 0].descriptorCount = 1;
        writeSets[i * 3 + 0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeSets[i * 3 + 0].pImageInfo = NULL;
        writeSets[i * 3 + 0].pBufferInfo = &uboInfo;
        writeSets[i * 3 + 0].pTexelBufferView = NULL;

        // Write for Sampler (Binding 1)
        writeSets[i * 3 + 1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSets[i * 3 + 1].pNext = NULL;
        writeSets[i * 3 + 1].dstSet = descSetLayoutData->vk_descriptor_set[i];
        writeSets[i * 3 + 1].dstBinding = 1;
        writeSets[i * 3 + 1].dstArrayElement = 0;
        writeSets[i * 3 + 1].descriptorCount = 1;
        writeSets[i * 3 + 1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeSets[i * 3 + 1].pImageInfo = &imageInfo[i]; // Use the correct imageInfo index
        writeSets[i * 3 + 1].pBufferInfo = NULL;
        writeSets[i * 3 + 1].pTexelBufferView = NULL;

        // Write for SSBO (Binding 2)
        writeSets[i * 3 + 2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSets[i * 3 + 2].pNext = NULL;
        writeSets[i * 3 + 2].dstSet = descSetLayoutData->vk_descriptor_set[i];
        writeSets[i * 3 + 2].dstBinding = 2;
        writeSets[i * 3 + 2].dstArrayElement = 0;
        writeSets[i * 3 + 2].descriptorCount = 1;
        writeSets[i * 3 + 2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeSets[i * 3 + 2].pImageInfo = NULL;
        writeSets[i * 3 + 2].pBufferInfo = &ssboInfo;
        writeSets[i * 3 + 2].pTexelBufferView = NULL;
    }

    // Update the descriptor sets
    rv("vkUpdateDescriptorSets (Bulk)");
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, NULL);
}