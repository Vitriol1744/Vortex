/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSurface.hpp"

#ifdef VT_PLATFORM_LINUX
    #define VK_USE_PLATFORM_X11_KHR
    #define GLFW_EXPOSE_NATIVE_X11
    #include <X11/Xlib-xcb.h>
    #include <vulkan/vulkan_xcb.h>
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #include <vulkan/vulkan_wayland.h>
#elifdef VT_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <vulkan/vulkan_win32.h>
#endif

#include <GLFW/glfw3native.h>

namespace Vortex
{
    void VulkanSurface::Create(Window*            window,
                               vk::PhysicalDevice physicalDevice)
    {
        VkInstance   vkInstance = (vk::Instance)VulkanContext::GetInstance();
        VkSurfaceKHR surface    = VK_NULL_HANDLE;

#ifdef VT_PLATFORM_LINUX
        if (Window::GetWindowSubsystem() == WindowSubsystem::eX11)
        {
            VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType
                = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext = VK_NULL_HANDLE;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.connection
                = XGetXCBConnection(glfwGetX11Display());
            surfaceCreateInfo.window
                = std::any_cast<::Window>(window->GetNativeHandle());

            vkCreateXcbSurfaceKHR(vkInstance, &surfaceCreateInfo,
                                  VK_NULL_HANDLE, &surface);
        }
        else if (Window::GetWindowSubsystem() == WindowSubsystem::eWayland)
        {
            VkWaylandSurfaceCreateInfoKHR createInfo{};
            createInfo.sType
                = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            createInfo.pNext   = VK_NULL_HANDLE;
            createInfo.flags   = 0;
            createInfo.display = glfwGetWaylandDisplay();
            createInfo.surface
                = std::any_cast<wl_surface*>(window->GetNativeHandle());

            VkCall(vk::Result(vkCreateWaylandSurfaceKHR(
                vkInstance, &createInfo, VK_NULL_HANDLE, &surface)));
        }
#elifdef VT_PLATFORM_WINDOWS
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext     = VK_NULL_HANDLE;
        createInfo.flags     = 0;
        createInfo.hinstance = GetModuleHandle(nullptr);
        createInfo.hwnd      = std::any_cast<HWND>(window->GetNativeHandle());

        VkCall(vk::Result(vkCreateWin32SurfaceKHR(vkInstance, &createInfo,
                                                  VK_NULL_HANDLE, &surface)));
#endif

        VtCoreAssert(surface != VK_NULL_HANDLE);
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
