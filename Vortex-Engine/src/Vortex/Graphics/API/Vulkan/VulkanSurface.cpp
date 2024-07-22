/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSurface.hpp"

#define VK_USE_PLATFORM_X11_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Vortex
{
    void VulkanSurface::Create(IWindow*           windowHandle,
                               vk::PhysicalDevice physicalDevice)
    {
        // TODO(v1tr10l7): Replace glfwCreateWindowSurface with native calls
        GLFWwindow* window
            = std::any_cast<GLFWwindow*>(windowHandle->GetNativeHandle());

        VkInstance   vkInstance = (vk::Instance)VulkanContext::GetInstance();
        VkSurfaceKHR surface;
        VkCall(vk::Result(
            glfwCreateWindowSurface(vkInstance, window, nullptr, &surface)));
        m_Surface       = vk::SurfaceKHR(surface);
        m_WindowHandle  = window;

        u32 formatCount = 0;
        VkCall(physicalDevice.getSurfaceFormatsKHR(m_Surface, &formatCount,
                                                   nullptr));
        std::vector<vk::SurfaceFormatKHR> surfaceFormats(formatCount);
        VkCall(physicalDevice.getSurfaceFormatsKHR(m_Surface, &formatCount,
                                                   surfaceFormats.data()));
        VtCoreAssertMsg(formatCount,
                        "Vulkan: Could not find any surface formats");
        m_SurfaceFormat = *(surfaceFormats.data());
        for (const auto& format : surfaceFormats)
        {
            if (format.format == vk::Format::eB8G8R8A8Srgb
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                m_SurfaceFormat = format;
        }

        u32 presentModeCount = 0;
        VkCall(physicalDevice.getSurfacePresentModesKHR(
            m_Surface, &presentModeCount, nullptr));
        m_PresentModes.resize(presentModeCount);
        VkCall(physicalDevice.getSurfacePresentModesKHR(
            m_Surface, &presentModeCount, m_PresentModes.data()));
    }
    void VulkanSurface::Destroy()
    {
        vk::Instance instance = vk::Instance(VulkanContext::GetInstance());
        instance.destroySurfaceKHR(m_Surface, nullptr);
    }

    const vk::SurfaceCapabilitiesKHR& VulkanSurface::GetCapabilities()
    {
        vk::PhysicalDevice physicalDevice = VulkanContext::GetPhysicalDevice();
        VkCall(physicalDevice.getSurfaceCapabilitiesKHR(
            m_Surface, &m_SurfaceCapabilities));

        return m_SurfaceCapabilities;
    }
}; // namespace Vortex
