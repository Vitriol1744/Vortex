/*
 * Created by v1tr10l7 on 07.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/UniformBuffer.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanUniformBuffer.hpp>

namespace Vortex
{
    ::Ref<UniformBuffer> UniformBuffer::Create(usize size)
    {
        return CreateRef<VulkanUniformBuffer>(size);
    }
}; // namespace Vortex
