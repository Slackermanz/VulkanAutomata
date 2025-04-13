#include "PipelineManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

VkResult createPipelineLayout(
    VkDevice logicalDevice,
    VkDescriptorSetLayout descriptorSetLayout,
    VK_Pipe* pipelineData,
    std::vector<VkResult>* vkres) {

    pipelineData->layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    nf(&pipelineData->layout_info);
    pipelineData->layout_info.setLayoutCount = 1;
    pipelineData->layout_info.pSetLayouts = &descriptorSetLayout;
    pipelineData->layout_info.pushConstantRangeCount = 0;
    pipelineData->layout_info.pPushConstantRanges = NULL;

    VkResult result = vkCreatePipelineLayout(logicalDevice, &pipelineData->layout_info, NULL, &pipelineData->vk_pipeline_layout);
    vr("vkCreatePipelineLayout", vkres, pipelineData->vk_pipeline_layout, result);

    return result;
}

VkResult createWorkGraphicsPipeline(
    VkDevice logicalDevice,
    VK_Pipe* pipelineData, // Contains layout handle
    VK_PipeInfo* pipelineConfig,
    VkRenderPass renderPass,
    std::vector<VkResult>* vkres) {

    pipelineData->gfx_pipe_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    nf(&pipelineData->gfx_pipe_info);
    pipelineData->gfx_pipe_info.stageCount = 2; // Vertex + Fragment
    pipelineData->gfx_pipe_info.pStages = pipelineConfig->p_shad_info; // From VK_PipeInfo
    pipelineData->gfx_pipe_info.pVertexInputState = &pipelineConfig->p_vtin_info;
    pipelineData->gfx_pipe_info.pInputAssemblyState = &pipelineConfig->p_inas_info;
    pipelineData->gfx_pipe_info.pTessellationState = NULL;
    pipelineData->gfx_pipe_info.pViewportState = &pipelineConfig->p_vprt_info;
    pipelineData->gfx_pipe_info.pRasterizationState = &pipelineConfig->p_rast_info;
    pipelineData->gfx_pipe_info.pMultisampleState = &pipelineConfig->p_msam_info;
    pipelineData->gfx_pipe_info.pDepthStencilState = NULL;
    pipelineData->gfx_pipe_info.pColorBlendState = &pipelineConfig->p_cbst_info;
    pipelineData->gfx_pipe_info.pDynamicState = NULL;
    pipelineData->gfx_pipe_info.layout = pipelineData->vk_pipeline_layout; // Use layout created previously
    pipelineData->gfx_pipe_info.renderPass = renderPass;
    pipelineData->gfx_pipe_info.subpass = 0;
    pipelineData->gfx_pipe_info.basePipelineHandle = VK_NULL_HANDLE;
    pipelineData->gfx_pipe_info.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineData->gfx_pipe_info, NULL, &pipelineData->vk_pipeline);
    vr("vkCreateGraphicsPipelines", vkres, pipelineData->vk_pipeline, result);

    return result;
}