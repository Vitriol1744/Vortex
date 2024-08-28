/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanAllocator.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    usize                VulkanContext::s_ContextCount   = 0;
    VulkanInstance       VulkanContext::s_VulkanInstance = {};
    VulkanPhysicalDevice VulkanContext::s_PhysicalDevice = {};
    VulkanDevice         VulkanContext::s_Device         = {};

    VulkanContext::VulkanContext(Window* window, bool vsync)
    {
        VtCoreTrace("Vulkan: Creating context for the window");
        if (s_ContextCount == 0)
        {
            s_VulkanInstance.Initialize();
            s_PhysicalDevice = VulkanPhysicalDevice::Pick();
            s_Device.Initialize(s_PhysicalDevice);
            VulkanAllocator::Initialize();
        }

        m_SwapChain.CreateSurface(window);
        m_SwapChain.Create(vsync);
        ++s_ContextCount;
    }

    VulkanContext::~VulkanContext()
    {
        VtCoreTrace("Vulkan: Destroying context of the window");

        m_SwapChain.Destroy();
        m_SwapChain.DestroySurface();
        --s_ContextCount;

        if (s_ContextCount == 0)
        {
            VulkanAllocator::Shutdown();
            s_Device.Destroy();
            s_VulkanInstance.Destroy();
        }
    }

    void VulkanContext::Present() { m_SwapChain.Present(); }
    void VulkanContext::OnResize() { m_SwapChain.OnResize(); }
}; // namespace Vortex
