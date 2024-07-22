/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"

namespace Vortex
{
    usize                VulkanContext::s_ContextCount   = 0;
    VulkanInstance       VulkanContext::s_VulkanInstance = {};
    VulkanPhysicalDevice VulkanContext::s_PhysicalDevice = {};

    VulkanContext::VulkanContext(class IWindow* window)
    {
        if (s_ContextCount == 0)
        {
            VtCoreAssert(glfwVulkanSupported() == GLFW_TRUE);
            s_VulkanInstance.Initialize();
            s_PhysicalDevice = VulkanPhysicalDevice::Pick();
        }
        m_Device.Initialize(s_PhysicalDevice);
        m_SwapChain.Initialize(m_Device);
        m_SwapChain.CreateSurface(window);
        u32 width, height;
        glfwGetFramebufferSize(m_SwapChain.GetSurface().GetNativeWindowHandle(),
                               (int*)&width, (int*)&height);
        m_SwapChain.Create(width, height, false);

        ++s_ContextCount;
    }
    VulkanContext::~VulkanContext()
    {
        m_SwapChain.Destroy();
        m_SwapChain.DestroySurface();
        m_Device.Destroy();
        --s_ContextCount;

        if (s_ContextCount == 0) s_VulkanInstance.Destroy();
    }

    void VulkanContext::Present() {}
    void VulkanContext::OnResize(u32 width, u32 height)
    {
        (void)width;
        (void)height;
    }

}; // namespace Vortex
