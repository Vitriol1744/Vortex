/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/GraphicsPipeline.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanShader.hpp>

namespace Vortex
{
    class VT_RENDERER_RHI_VULKAN_API VulkanGraphicsPipeline final
        : public GraphicsPipeline
    {
      public:
        VulkanGraphicsPipeline(
            const GraphicsPipelineSpecification& specification);
        virtual ~VulkanGraphicsPipeline();

        inline operator vk::Pipeline() const { return m_Pipeline; }
        inline vk::PipelineLayout  Layout() const { return m_PipelineLayout; }
        inline ::Ref<VulkanShader> Shader() const { return m_Shader; }

      private:
        vk::Pipeline        m_Pipeline       = VK_NULL_HANDLE;
        vk::PipelineLayout  m_PipelineLayout = VK_NULL_HANDLE;
        ::Ref<VulkanShader> m_Shader         = nullptr;
    };
}; // namespace Vortex
