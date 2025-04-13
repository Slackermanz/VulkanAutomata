#include "PipelineManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf)

// --- Existing functions createPipelineLayout and createWorkGraphicsPipeline remain here ---

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

// --- NEW FUNCTION ADDED BELOW ---

void setupPipelineInfoDefaults(
    VK_PipeInfo* pipelineInfo,   // Output: Struct to be filled
    VK_RPConfig* renderPassConfig // Input: Contains viewport and scissor rect
) {
    // Rasterization State
    pipelineInfo->p_rast_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    nf(&pipelineInfo->p_rast_info);
    pipelineInfo->p_rast_info.depthClampEnable = VK_FALSE;
    pipelineInfo->p_rast_info.rasterizerDiscardEnable = VK_FALSE;
    pipelineInfo->p_rast_info.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineInfo->p_rast_info.cullMode = VK_CULL_MODE_NONE;
    pipelineInfo->p_rast_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineInfo->p_rast_info.depthBiasEnable = VK_FALSE;
    pipelineInfo->p_rast_info.depthBiasConstantFactor = 0.0f;
    pipelineInfo->p_rast_info.depthBiasClamp = 0.0f;
    pipelineInfo->p_rast_info.depthBiasSlopeFactor = 0.0f;
    pipelineInfo->p_rast_info.lineWidth = 1.0f;

    // Viewport State (uses viewport/scissor from renderPassConfig)
    pipelineInfo->p_vprt_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    nf(&pipelineInfo->p_vprt_info);
    pipelineInfo->p_vprt_info.viewportCount = 1;
    pipelineInfo->p_vprt_info.pViewports = &renderPassConfig->vk_viewport;
    pipelineInfo->p_vprt_info.scissorCount = 1;
    pipelineInfo->p_vprt_info.pScissors = &renderPassConfig->rect2D;

    // Multisample State
    pipelineInfo->p_msam_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    nf(&pipelineInfo->p_msam_info);
    pipelineInfo->p_msam_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo->p_msam_info.sampleShadingEnable = VK_FALSE;
    pipelineInfo->p_msam_info.minSampleShading = 0.0f;
    pipelineInfo->p_msam_info.pSampleMask = NULL;
    pipelineInfo->p_msam_info.alphaToCoverageEnable = VK_FALSE;
    pipelineInfo->p_msam_info.alphaToOneEnable = VK_FALSE;

    // Vertex Input State (no vertex input for this full-screen shader)
    pipelineInfo->p_vtin_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    nf(&pipelineInfo->p_vtin_info);
    pipelineInfo->p_vtin_info.vertexBindingDescriptionCount = 0;
    pipelineInfo->p_vtin_info.pVertexBindingDescriptions = NULL;
    pipelineInfo->p_vtin_info.vertexAttributeDescriptionCount = 0;
    pipelineInfo->p_vtin_info.pVertexAttributeDescriptions = NULL;

    // Input Assembly State (drawing a single triangle list)
    pipelineInfo->p_inas_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    nf(&pipelineInfo->p_inas_info);
    pipelineInfo->p_inas_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInfo->p_inas_info.primitiveRestartEnable = VK_FALSE;

    // Color Blend Attachment State (no blending needed)
    pipelineInfo->p_cbat_info.blendEnable = VK_FALSE;
    pipelineInfo->p_cbat_info.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineInfo->p_cbat_info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineInfo->p_cbat_info.colorBlendOp = VK_BLEND_OP_ADD;
    pipelineInfo->p_cbat_info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineInfo->p_cbat_info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineInfo->p_cbat_info.alphaBlendOp = VK_BLEND_OP_ADD;
    pipelineInfo->p_cbat_info.colorWriteMask = 15; // VK_COLOR_COMPONENT_R_BIT | G | B | A

    // Color Blend State
    pipelineInfo->p_cbst_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    nf(&pipelineInfo->p_cbst_info);
    pipelineInfo->p_cbst_info.logicOpEnable = VK_FALSE;
    pipelineInfo->p_cbst_info.logicOp = VK_LOGIC_OP_NO_OP;
    pipelineInfo->p_cbst_info.attachmentCount = 1;
    pipelineInfo->p_cbst_info.pAttachments = &pipelineInfo->p_cbat_info;
    pipelineInfo->p_cbst_info.blendConstants[0] = 1.0f;
    pipelineInfo->p_cbst_info.blendConstants[1] = 1.0f;
    pipelineInfo->p_cbst_info.blendConstants[2] = 1.0f;
    pipelineInfo->p_cbst_info.blendConstants[3] = 1.0f;

    // Note: Shader stages (p_shad_info) are set separately after loading shaders.
}