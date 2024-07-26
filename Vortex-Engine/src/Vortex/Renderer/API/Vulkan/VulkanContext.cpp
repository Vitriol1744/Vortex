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
    usize          VulkanContext::s_ContextCount   = 0;
    VulkanInstance VulkanContext::s_VulkanInstance = {};

    VulkanContext::VulkanContext(Window* window)
    {
        VtCoreTrace("Vulkan: Creating context for the window");
        if (s_ContextCount == 0)
        {
            VtCoreAssert(glfwVulkanSupported() == GLFW_TRUE);
            s_VulkanInstance.Initialize();
        }

        (void)window;
        ++s_ContextCount;
    }

    VulkanContext::~VulkanContext()
    {
        VtCoreTrace("Vulkan: Destroying context of the window");
        --s_ContextCount;

        if (s_ContextCount == 0) s_VulkanInstance.Destroy();
    }
}; // namespace Vortex
