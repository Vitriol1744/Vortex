/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanSurface.hpp>
#include <Vortex/Window/Wayland/Wayland.hpp>

#ifdef VT_PLATFORM_LINUX
    #define VK_USE_PLATFORM_X11_KHR
    #include <Vortex/Window/X11/X11.hpp>
    #include <X11/Xlib-xcb.h>
    #include <vulkan/vulkan_wayland.h>
    #include <vulkan/vulkan_xcb.h>
#elifdef VT_PLATFORM_WINDOWS
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <vulkan/vulkan_win32.h>
#endif

namespace Vortex
{
    void VulkanSurface::Create(class Window*      window,
                               vk::PhysicalDevice physicalDevice)
    {
        VkInstance vkInstance = (vk::Instance)VulkanRenderer::VulkanInstance();
        VkSurfaceKHR surface  = VK_NULL_HANDLE;

#ifdef VT_PLATFORM_LINUX
        if (Window::WindowSubsystem() == WindowSubsystem::eX11)
        {
            VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType
                = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext      = VK_NULL_HANDLE;
            surfaceCreateInfo.flags      = 0;
            surfaceCreateInfo.connection = X11::Connection();
            surfaceCreateInfo.window
                = std::any_cast<xcb_window_t>(window->NativeHandle());

            VkCall(vk::Result(vkCreateXcbSurfaceKHR(
                vkInstance, &surfaceCreateInfo, VK_NULL_HANDLE, &surface)));
        }
        else if (Window::WindowSubsystem() == WindowSubsystem::eWayland)
        {
            VkWaylandSurfaceCreateInfoKHR createInfo{};
            createInfo.sType
                = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            createInfo.pNext   = VK_NULL_HANDLE;
            createInfo.flags   = 0;
            createInfo.display = Wayland::Display();
            createInfo.surface
                = std::any_cast<wl_surface*>(window->NativeHandle());

            VkCall(vk::Result(vkCreateWaylandSurfaceKHR(
                vkInstance, &createInfo, VK_NULL_HANDLE, &surface)));
        }
#elifdef VT_PLATFORM_WINDOWS
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext     = VK_NULL_HANDLE;
        createInfo.flags     = 0;
        createInfo.hinstance = GetModuleHandle(nullptr);
        createInfo.hwnd      = std::any_cast<HWND>(window->NativeHandle());

        VkCall(vk::Result(vkCreateWin32SurfaceKHR(vkInstance, &createInfo,
                                                  VK_NULL_HANDLE, &surface)));
#endif

        VtCoreAssert(surface != VK_NULL_HANDLE);
        m_Surface       = vk::SurfaceKHR(surface);
        m_WindowHandle  = window;

        u32 formatCount = 0;
        VkCall(physicalDevice.getSurfaceFormatsKHR(m_Surface, &formatCount,
                                                   nullptr));
        Vector<vk::SurfaceFormatKHR> surfaceFormats(formatCount);
        VkCall(physicalDevice.getSurfaceFormatsKHR(m_Surface, &formatCount,
                                                   surfaceFormats.Raw()));
        VtCoreAssertMsg(formatCount,
                        "Vulkan: Could not find any surface formats");
        m_SurfaceFormat = *(surfaceFormats.Raw());
        for (const auto& format : surfaceFormats)
        {
            if (format.format == vk::Format::eB8G8R8A8Srgb
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                m_SurfaceFormat = format;
        }

        u32 presentModeCount = 0;
        VkCall(physicalDevice.getSurfacePresentModesKHR(
            m_Surface, &presentModeCount, nullptr));
        m_PresentModes.Resize(presentModeCount);
        VkCall(physicalDevice.getSurfacePresentModesKHR(
            m_Surface, &presentModeCount, m_PresentModes.Raw()));
    }
    void VulkanSurface::Destroy()
    {
        vk::Instance instance = vk::Instance(VulkanRenderer::VulkanInstance());
        instance.destroySurfaceKHR(m_Surface, nullptr);
    }

    const vk::SurfaceCapabilitiesKHR& VulkanSurface::Capabilities()
    {
        vk::PhysicalDevice physicalDevice = VulkanRenderer::PhysicalDevice();
        VkCall(physicalDevice.getSurfaceCapabilitiesKHR(
            m_Surface, &m_SurfaceCapabilities));

        return m_SurfaceCapabilities;
    }
}; // namespace Vortex
