/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/SwapChain.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{
    Ref<SwapChain> SwapChain::Create(Window* window, bool vsync)
    {
        return CreateRef<VulkanSwapChain>(window, vsync);
    }
}; // namespace Vortex
