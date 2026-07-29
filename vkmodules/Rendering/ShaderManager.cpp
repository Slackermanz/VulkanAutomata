#include "ShaderManager.h"
#include "../Utils/Logger.h" // For logging helpers (vr, rv, nf, iv, ov)
#include <fstream> // For file reading
#include <limits>

// Note: getShaderCodeInfo is now implicitly handled here

VkResult loadAndCreateShaderModule(
    VkDevice logicalDevice,
    const std::string& filename,
    ShaderData* shaderData, // Output struct
    std::vector<VkResult>* vkres) {

    // 1. Read shader file
    rv("Reading Shader File");
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        ov("Failed to open shader file", filename);
        return VK_ERROR_INITIALIZATION_FAILED; // Or another appropriate error
    }

    std::streampos endPos = file.tellg();
    if (endPos == std::streampos(-1) || endPos <= std::streampos(0)) {
        ov("Invalid shader file size", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::streamoff fileBytes = static_cast<std::streamoff>(endPos);
    if (fileBytes > std::numeric_limits<std::streamsize>::max()) {
        ov("Shader file too large", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (fileBytes % 4 != 0) {
        ov("Invalid SPIR-V size (not multiple of 4)", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    size_t fileSize = static_cast<size_t>(fileBytes);
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    if (file.gcount() != static_cast<std::streamsize>(fileSize)) {
        ov("Failed reading complete shader file", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    file.close();

    // Store info in ShaderData struct
    shaderData->SC_info.shaderFilename = filename;
    shaderData->SC_info.shaderData = buffer;
    shaderData->SC_info.shaderBytes = buffer.size();
    shaderData->SC_info.shaderBytesValid = (shaderData->SC_info.shaderBytes % 4 == 0);

    iv("Shader Filename", shaderData->SC_info.shaderFilename, 0); // Using index 0 for simplicity
    iv("Shader Bytes", shaderData->SC_info.shaderBytes, 0);
    iv("Shader Bytes Valid", (shaderData->SC_info.shaderBytesValid ? "TRUE" : "FALSE"), 0);

    if (!shaderData->SC_info.shaderBytesValid) {
        ov("Invalid SPIR-V size (not multiple of 4)", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // 2. Create Shader Module Info
    shaderData->vk_SM_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    nf(&shaderData->vk_SM_info);
    shaderData->vk_SM_info.codeSize = shaderData->SC_info.shaderBytes;
    shaderData->vk_SM_info.pCode = reinterpret_cast<const uint32_t*>(shaderData->SC_info.shaderData.data());

    // 3. Create Shader Module
    VkResult result = vkCreateShaderModule(logicalDevice, &shaderData->vk_SM_info, NULL, &shaderData->vk_shader_module);
    vr("vkCreateShaderModule", vkres, shaderData->vk_shader_module, result);

    // Note: Shader stage (Vertex/Fragment) needs to be set by the caller
    // after calling this function, as it's context-dependent.
    shaderData->stage_bits = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; // Indicate stage not set yet

    return result;
}