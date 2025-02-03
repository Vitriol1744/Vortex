/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/VertexBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.hpp"

namespace Vortex
{

    Ref<VertexBuffer> VertexBuffer::Create(void* data, usize size)
    {
        return CreateRef<VulkanVertexBuffer>(data, size);
    }
}; // namespace Vortex
