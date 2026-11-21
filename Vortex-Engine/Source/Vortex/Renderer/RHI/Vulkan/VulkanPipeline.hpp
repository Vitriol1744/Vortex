/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

namespace Vortex::Vulkan
{
    class VulkanBindGroup final : public RHI::BindGroup
    {
      public:
        explicit VulkanBindGroup(const RHI::BindGroupDesc& desc);
        ~VulkanBindGroup() override;

        inline vk::DescriptorSet Handle() const { return m_Set; }

      private:
        vk::DescriptorSet  m_Set  = VK_NULL_HANDLE;
        vk::DescriptorPool m_Pool = VK_NULL_HANDLE;
    };

    class VulkanPipeline final : public RHI::Pipeline
    {
      public:
        explicit VulkanPipeline(const RHI::GraphicsPipelineDesc& desc);
        explicit VulkanPipeline(const RHI::ComputePipelineDesc& desc);
        ~VulkanPipeline() override;

        inline vk::Pipeline       Handle() const { return m_Pipeline; }
        inline vk::PipelineBindPoint BindPoint() const
        {
            return m_Type == RHI::PipelineType::eCompute
                     ? vk::PipelineBindPoint::eCompute
                     : vk::PipelineBindPoint::eGraphics;
        }
        inline vk::PipelineLayout Layout() const { return m_Layout; }
        /// Stages the push constant range covers.
        inline vk::ShaderStageFlags PushConstantStages() const
        {
            return m_PushConstantStages;
        }

      private:
        vk::Pipeline         m_Pipeline = VK_NULL_HANDLE;
        vk::PipelineLayout   m_Layout   = VK_NULL_HANDLE;
        vk::ShaderStageFlags m_PushConstantStages{};

        /// Pipeline layout from the shader's set layouts and push constants.
        void                 CreateLayout(class VulkanShader& shader);
    };
} // namespace Vortex::Vulkan
