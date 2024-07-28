/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

namespace Vortex
{
    VulkanShader::VulkanShader()
    {
        std::vector<u8> vertexShaderCode;
        ReadShaderCode("vert.spv", vertexShaderCode);
        std::vector<u8> fragmentShaderCode;
        ReadShaderCode("frag.spv", fragmentShaderCode);

        vk::ShaderModuleCreateInfo vertexShaderInfo{};
        vertexShaderInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
        vertexShaderInfo.codeSize = vertexShaderCode.size();
        vertexShaderInfo.pCode
            = reinterpret_cast<const u32*>(vertexShaderCode.data());

        vk::ShaderModuleCreateInfo fragmentShaderInfo{};
        fragmentShaderInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        fragmentShaderInfo.codeSize = fragmentShaderCode.size();
        fragmentShaderInfo.pCode
            = reinterpret_cast<const u32*>(fragmentShaderCode.data());

        vk::Device device = VulkanContext::GetDevice();
        VkCall(device.createShaderModule(&vertexShaderInfo, VK_NULL_HANDLE,
                                         &m_VertexShader));
        VkCall(device.createShaderModule(&fragmentShaderInfo, VK_NULL_HANDLE,
                                         &m_FragmentShader));
    }
    VulkanShader::~VulkanShader()
    {
        vk::Device device = VulkanContext::GetDevice();
        device.destroyShaderModule(m_VertexShader, VK_NULL_HANDLE);
        device.destroyShaderModule(m_FragmentShader, VK_NULL_HANDLE);
    }

    void VulkanShader::ReadShaderCode(std::string_view path,
                                      std::vector<u8>& buffer)
    {
        std::ifstream file(path.data(), std::ios::ate | std::ios::binary);

        // TODO(v1tr10l7): We shouldn't crash here
        VtCoreAssert(file.is_open());

        usize fileSize = file.tellg();
        buffer.resize(fileSize);

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    }
}; // namespace Vortex
