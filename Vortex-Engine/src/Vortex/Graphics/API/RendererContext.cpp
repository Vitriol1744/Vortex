/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/API/RendererContext.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"

namespace Vortex
{
    Ref<RendererContext> RendererContext::Create(Window* window)
    {
        return CreateRef<VulkanContext>(window);
    }
}; // namespace Vortex
