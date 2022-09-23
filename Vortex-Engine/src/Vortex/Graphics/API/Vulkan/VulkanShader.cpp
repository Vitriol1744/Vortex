//
// Created by Vitriol1744 on 15.09.2022.
//
#include "VulkanShader.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    VulkanShader::VulkanShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, bool binaries)
    {
        VT_CORE_ASSERT_MSG(binaries == true, "Vortex currently doesn't support compiling shaders at runtime!");

        auto vertexShaderCode = ReadFile(vertexShaderPath);
        auto fragmentShaderCode = ReadFile(fragmentShaderPath);

        vertexShader = CreateShaderModule(vertexShaderCode);
        fragmentShader = CreateShaderModule(fragmentShaderCode);

        VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
        vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderStageCreateInfo.module = vertexShader;
        vertexShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
        fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderStageCreateInfo.module = fragmentShader;
        fragmentShaderStageCreateInfo.pName = "main";

        shaderStages = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };
    }
    VulkanShader::~VulkanShader()
    {
        VkDevice& device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;

        vkDestroyShaderModule(device, vertexShader, allocator);
        vkDestroyShaderModule(device, fragmentShader, allocator);
    }

    std::vector<char> VulkanShader::ReadFile(std::string_view filepath)
    {
        std::ifstream file(filepath.data(), std::ios::binary | std::ios::ate);

        VT_CORE_ASSERT_MSG(file.is_open(), "Failed to open Shader file!");

        size_t fileSize = file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
    VkShaderModule VulkanShader::CreateShaderModule(std::vector<char>& shaderCode)
    {
        VkShaderModuleCreateInfo shaderCreateInfo = {};
        shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderCreateInfo.codeSize = shaderCode.size();
        shaderCreateInfo.pCode = reinterpret_cast<const uint32*>(shaderCode.data());

        VkShaderModule shaderModule;
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;

        VkCall(vkCreateShaderModule(device, &shaderCreateInfo, allocator, &shaderModule));

        return shaderModule;
    }
}
