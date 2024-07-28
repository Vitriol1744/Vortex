/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    usize                VulkanContext::s_ContextCount   = 0;
    VulkanInstance       VulkanContext::s_VulkanInstance = {};
    VulkanPhysicalDevice VulkanContext::s_PhysicalDevice = {};
    VulkanDevice         VulkanContext::s_Device         = {};

    VulkanContext::VulkanContext(Window* window)
    {
        VtCoreTrace("Vulkan: Creating context for the window");
        if (s_ContextCount == 0)
        {
            VtCoreAssert(glfwVulkanSupported() == GLFW_TRUE);
            s_VulkanInstance.Initialize();
            s_PhysicalDevice = VulkanPhysicalDevice::Pick();
            s_Device.Initialize(s_PhysicalDevice);
        }

        m_SwapChain.CreateSurface(window);
        u32 width, height;
        glfwGetFramebufferSize(m_SwapChain.GetSurface().GetNativeWindowHandle(),
                               reinterpret_cast<i32*>(&width),
                               reinterpret_cast<i32*>(&height));
        m_SwapChain.Create(width, height, false);
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
            s_Device.Destroy();
            s_VulkanInstance.Destroy();
        }
    }

    void VulkanContext::Present() { m_SwapChain.Present(); }
    void VulkanContext::OnResize(u32 width, u32 height)
    {
        m_SwapChain.OnResize(width, height);
    }
}; // namespace Vortex
