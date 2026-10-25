/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/IndexBuffer.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanIndexBuffer.hpp>

namespace Vortex
{

    ::Ref<IndexBuffer> IndexBuffer::Create(u32* data, usize size)
    {
        return CreateRef<VulkanIndexBuffer>(data, size);
    }
}; // namespace Vortex
