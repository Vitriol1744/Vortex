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

    VulkanContext::VulkanContext(Window* window)
    {
        VtCoreTrace("Vulkan: Creating context for the window");
        if (s_ContextCount == 0)
        {
            VtCoreAssert(glfwVulkanSupported() == GLFW_TRUE);
            s_VulkanInstance.Initialize();
            s_PhysicalDevice = VulkanPhysicalDevice::Pick();
        }

        m_Device.Initialize(s_PhysicalDevice);
        (void)window;
        ++s_ContextCount;
    }

    VulkanContext::~VulkanContext()
    {
        VtCoreTrace("Vulkan: Destroying context of the window");

        m_Device.Destroy();
        --s_ContextCount;

        if (s_ContextCount == 0) s_VulkanInstance.Destroy();
    }
}; // namespace Vortex
