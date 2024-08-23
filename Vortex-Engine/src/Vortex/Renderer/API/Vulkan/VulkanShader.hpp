/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.hpp"

namespace Vortex
{
    class VT_API VulkanShader final : public Shader
    {
      public:
        struct UniformDescriptor
        {
            std::string             Name;
            u32                     BindingPoint;
            u32                     DescriptorCount;
            u32                     Size;
            vk::ShaderStageFlagBits ShaderStage;
        };
        struct ImageSampler
        {
            std::string             Name;
            u32                     BindingPoint    = 0;
            u32                     DescriptorCount = 1;
            vk::ShaderStageFlagBits ShaderStage;
        };

        struct DescriptorSet
        {
            std::unordered_map<u32, UniformDescriptor> UniformDescriptors;
            std::unordered_map<u32, ImageSampler>      ImageSamplers;

            vk::DescriptorPool                         Pool = VK_NULL_HANDLE;
            std::vector<vk::DescriptorPoolSize>        PoolSizes;

            std::vector<vk::DescriptorSet>             Sets;
            std::unordered_map<std::string, vk::WriteDescriptorSet>
                WriteDescriptorSets;
        };

        VulkanShader(std::string_view path);
        VulkanShader(std::string_view vertexPath,
                     std::string_view fragmentPath);
        virtual ~VulkanShader();

        void SetUniform(const std::string&       name,
                        Ref<VulkanUniformBuffer> buffer);
        void SetUniform(const std::string& name, Ref<VulkanTexture2D> texture);

        inline vk::ShaderModule GetVertexShader() const
        {
            return m_VertexShader;
        }
        inline vk::ShaderModule GetFragmentShader() const
        {
            return m_FragmentShader;
        }

        inline const auto& GetDescriptorSetLayouts() const
        {
            return m_DescriptorSetLayouts;
        }
        inline auto& GetDescriptorSets() { return m_DescriptorSets; }

      private:
        vk::ShaderModule m_VertexShader   = VK_NULL_HANDLE;
        vk::ShaderModule m_FragmentShader = VK_NULL_HANDLE;

        std::unordered_map<u32, DescriptorSet> m_DescriptorSets;
        std::vector<vk::DescriptorSetLayout>   m_DescriptorSetLayouts;

        std::unordered_map<vk::ShaderStageFlagBits, std::string>
             Preprocess(std::string_view shaderCode);
        void Reflect(std::string_view path, vk::ShaderStageFlagBits stage,
                     const std::vector<u32>& shaderBinary);

        static std::string ReadShaderCode(std::string_view path);

        static void        CompileShaderCode(std::string_view  source,
                                             std::string_view  path,
                                             std::vector<u32>& outputBinary);
    };
}; // namespace Vortex
