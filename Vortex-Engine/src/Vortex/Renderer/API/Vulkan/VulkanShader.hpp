/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"

namespace Vortex
{
    class VT_API VulkanShader final : public Shader
    {
      public:
        VulkanShader(std::string_view vertexPath,
                     std::string_view fragmentPath);
        virtual ~VulkanShader();

        inline vk::ShaderModule GetVertexShader() const
        {
            return m_VertexShader;
        }
        inline vk::ShaderModule GetFragmentShader() const
        {
            return m_FragmentShader;
        }

      private:
        vk::ShaderModule m_VertexShader   = VK_NULL_HANDLE;
        vk::ShaderModule m_FragmentShader = VK_NULL_HANDLE;

        static void      ReadShaderCode(std::string_view path,
                                        std::vector<u8>& buffer);
    };
}; // namespace Vortex
