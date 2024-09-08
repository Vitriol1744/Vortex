/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"

namespace Vortex
{
    VulkanContext::VulkanContext(Window* window, bool vsync)
    {
        VtCoreTrace("Vulkan: Creating context for the window");

        m_SwapChain.CreateSurface(window);
        m_SwapChain.Create(vsync);
    }

    VulkanContext::~VulkanContext()
    {
        VtCoreTrace("Vulkan: Destroying context of the window");

        m_SwapChain.Destroy();
        m_SwapChain.DestroySurface();
    }

    void VulkanContext::Present() { m_SwapChain.Present(); }
    void VulkanContext::OnResize() { m_SwapChain.OnResize(); }
}; // namespace Vortex
