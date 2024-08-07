/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

// #include <libshaderc_util/file_finder.h>
#include <shaderc/shaderc.hpp>
#include <spirv-tools/libspirv.h>
#include <spirv_cross/spirv_glsl.hpp>

namespace Vortex
{
    shaderc::Compiler          s_Compiler;

    static shaderc_shader_kind ToShadercStage(vk::ShaderStageFlagBits stage)
    {
        switch (stage)
        {
            case vk::ShaderStageFlagBits::eVertex: return shaderc_vertex_shader;
            case vk::ShaderStageFlagBits::eFragment:
                return shaderc_fragment_shader;

            default: break;
        }

        VtCoreAssertMsg(false, "Unknown shader stage");
        return static_cast<shaderc_shader_kind>(0);
    }

    VulkanShader::VulkanShader(std::string_view vertexPath,
                               std::string_view fragmentPath)
    {
        std::vector<u32> vertexShaderCode;
        std::vector<u32> fragmentShaderCode;
        CompileShaderCode(vertexPath, vk::ShaderStageFlagBits::eVertex,
                          vertexShaderCode);
        CompileShaderCode(fragmentPath, vk::ShaderStageFlagBits::eFragment,
                          fragmentShaderCode);

        vk::ShaderModuleCreateInfo vertexShaderInfo{};
        vertexShaderInfo.sType    = vk::StructureType::eShaderModuleCreateInfo;
        vertexShaderInfo.codeSize = vertexShaderCode.size() * sizeof(u32);
        vertexShaderInfo.pCode
            = reinterpret_cast<const u32*>(vertexShaderCode.data());

        vk::ShaderModuleCreateInfo fragmentShaderInfo{};
        fragmentShaderInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        fragmentShaderInfo.codeSize = fragmentShaderCode.size() * sizeof(u32);
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

    std::string VulkanShader::ReadShaderCode(std::string_view path)
    {
        std::ifstream ifs(path.data());
        VtCoreAssert(ifs.is_open());

        std::stringstream source;
        source << ifs.rdbuf();

        return source.str();
    }

    void VulkanShader::CompileShaderCode(std::string_view        path,
                                         vk::ShaderStageFlagBits stage,
                                         std::vector<u32>&       outputBinary)
    {
        std::string             source = ReadShaderCode(path);

        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan,
                                     shaderc_env_version_vulkan_1_2);

#ifdef VT_DIST
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

        shaderc::SpvCompilationResult module = s_Compiler.CompileGlslToSpv(
            source.data(), ToShadercStage(stage), path.data(), options);

        VtCoreAssert(module.GetCompilationStatus()
                     == shaderc_compilation_status_success);

        outputBinary = std::vector<u32>(module.cbegin(), module.cend());
    }
}; // namespace Vortex
