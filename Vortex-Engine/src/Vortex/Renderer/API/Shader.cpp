/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

namespace Vortex
{
    Ref<Shader> Shader::Create(std::string_view vertexPath,
                               std::string_view fragmentPath)
    {
        return CreateRef<VulkanShader>(vertexPath, fragmentPath);
    }
}; // namespace Vortex
