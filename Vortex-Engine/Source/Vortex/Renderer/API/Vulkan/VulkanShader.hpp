/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/UnorderedMap.hpp>

#include <Vortex/Renderer/API/Shader.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.hpp>

namespace Vortex
{
    class VT_API VulkanShader final : public Shader
    {
      public:
        struct UniformDescriptor
        {
            String                  Name;
            u32                     BindingPoint;
            u32                     DescriptorCount;
            u32                     Size;
            vk::ShaderStageFlagBits ShaderStage;
        };
        struct ImageSampler
        {
            String                  Name;
            u32                     BindingPoint    = 0;
            u32                     DescriptorCount = 1;
            vk::ShaderStageFlagBits ShaderStage;
        };

        struct DescriptorSet
        {
            UnorderedMap<u32, UniformDescriptor>         UniformDescriptors;
            UnorderedMap<u32, ImageSampler>              ImageSamplers;

            vk::DescriptorPool                           Pool = VK_NULL_HANDLE;
            Vector<vk::DescriptorPoolSize>               PoolSizes;

            Vector<vk::DescriptorSet>                    Sets;
            UnorderedMap<String, vk::WriteDescriptorSet> WriteDescriptorSets;
        };

        VulkanShader(PathView path);
        VulkanShader(PathView vertexPath, PathView fragmentPath);
        virtual ~VulkanShader();

        void SetUniform(const String&        name,
                        ::Ref<UniformBuffer> buffer) override;
        void SetUniform(const String& name, ::Ref<Texture2D> texture) override;

        inline vk::ShaderModule VertexShader() const { return m_VertexShader; }
        inline vk::ShaderModule FragmentShader() const
        {
            return m_FragmentShader;
        }

        inline const auto& DescriptorSetLayouts() const
        {
            return m_DescriptorSetLayouts;
        }
        inline auto& DescriptorSets() { return m_DescriptorSets; }

      private:
        vk::ShaderModule                 m_VertexShader   = VK_NULL_HANDLE;
        vk::ShaderModule                 m_FragmentShader = VK_NULL_HANDLE;

        UnorderedMap<u32, DescriptorSet> m_DescriptorSets;
        Vector<vk::DescriptorSetLayout>  m_DescriptorSetLayouts;

        UnorderedMap<vk::ShaderStageFlagBits, String>
                      Preprocess(StringView shaderCode);
        void          Reflect(StringView path, vk::ShaderStageFlagBits stage,
                              const Vector<u32>& shaderBinary);

        static String ReadShaderCode(StringView path);

        static void   CompileShaderCode(StringView source, StringView path,
                                        vk::ShaderStageFlagBits stage,
                                        Vector<u32>&            outputBinary);
    };
}; // namespace Vortex
