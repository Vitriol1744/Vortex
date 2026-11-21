/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

import Prism.Containers.Span;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Utility.Expected;

namespace Vortex::Vulkan
{
    class VulkanBindGroupLayout final : public RHI::BindGroupLayout
    {
      public:
        explicit VulkanBindGroupLayout(const RHI::BindGroupLayoutDesc& desc);
        ~VulkanBindGroupLayout() override;

        inline vk::DescriptorSetLayout Handle() const { return m_Layout; }

      private:
        vk::DescriptorSetLayout m_Layout = VK_NULL_HANDLE;
    };

    class VulkanShader final : public RHI::Shader
    {
      public:
        struct Module
        {
            RHI::ShaderStage Stage  = RHI::ShaderStage::eNone;
            vk::ShaderModule Handle = VK_NULL_HANDLE;
        };

        /// Compiles, reflects and creates the modules.
        static Expected<::Ref<VulkanShader>, String>
        Create(const RHI::ShaderDesc& desc);

        ~VulkanShader() override;

        ::Ref<RHI::BindGroupLayout> SetLayout(u32 set) const override;

        inline const Vector<Module>& Modules() const { return m_Modules; }
        /// One layout per set index (empty sets use the device's empty
        /// layout), as a pipeline layout wants them.
        inline const Vector<vk::DescriptorSetLayout>& SetLayoutHandles() const
        {
            return m_SetLayoutHandles;
        }

        VulkanShader() = default;

      private:
        Vector<Module>                        m_Modules;
        Vector<::Ref<VulkanBindGroupLayout>>  m_SetLayouts;
        Vector<vk::DescriptorSetLayout>       m_SetLayoutHandles;
    };
} // namespace Vortex::Vulkan
