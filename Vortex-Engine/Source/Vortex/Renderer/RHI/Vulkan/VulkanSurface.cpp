/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

// Platform C headers first: they must precede every `import` in this TU
// (VulkanCommon.hpp has imports), and the VK_USE_PLATFORM_* macros must be
// defined before <vulkan/vulkan.h>.
#include <Vortex/Core/Platform.hpp>

#if defined(VT_PLATFORM_ANDROID)
    #define VK_USE_PLATFORM_ANDROID_KHR
    #include <android/native_window.h>
#elif defined(VT_PLATFORM_LINUX)
    #define VK_USE_PLATFORM_XCB_KHR
    #define VK_USE_PLATFORM_WAYLAND_KHR
    #include <X11/Xlib-xcb.h>
    #include <wayland-client.h>
#elif defined(VT_PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <Windows.h>
#endif
#include <vulkan/vulkan.h>

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSurface.hpp>

#include <Vortex/Window/Window.hpp>
#if defined(VT_PLATFORM_LINUX) && !defined(VT_PLATFORM_ANDROID)
    #include <Vortex/Window/Wayland/Wayland.hpp>
    #include <Vortex/Window/X11/X11.hpp>
#endif

#include <any>

namespace Vortex::Vulkan
{
    bool VulkanSurface::Create(class Window* window)
    {
        VtCoreAssertMsg(window, "Vulkan: Surface for a null window");
        auto&        device   = VulkanDevice::Get();
        VkInstance   instance = device.Instance();
        VkSurfaceKHR surface  = VK_NULL_HANDLE;
        VkResult     result   = VK_ERROR_INITIALIZATION_FAILED;

#if defined(VT_PLATFORM_ANDROID)
        VkAndroidSurfaceCreateInfoKHR info{};
        info.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        info.window = std::any_cast<ANativeWindow*>(window->NativeHandle());
        result = vkCreateAndroidSurfaceKHR(instance, &info, nullptr, &surface);
#elif defined(VT_PLATFORM_LINUX)
        if (Window::WindowSubsystem() == WindowSubsystem::eWayland)
        {
            VkWaylandSurfaceCreateInfoKHR info{};
            info.sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            info.display = Wayland::Display();
            info.surface = std::any_cast<wl_surface*>(window->NativeHandle());
            result = vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &surface);
        }
        else
        {
            VkXcbSurfaceCreateInfoKHR info{};
            info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            info.connection = X11::Connection();
            info.window = std::any_cast<xcb_window_t>(window->NativeHandle());
            result      = vkCreateXcbSurfaceKHR(instance, &info, nullptr, &surface);
        }
#elif defined(VT_PLATFORM_WINDOWS)
        VkWin32SurfaceCreateInfoKHR info{};
        info.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        info.hinstance = GetModuleHandle(nullptr);
        info.hwnd      = std::any_cast<HWND>(window->NativeHandle());
        result = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface);
#endif
        if (result != VK_SUCCESS || !surface)
        {
            VtCoreError("Vulkan: Failed to create a window surface: {}",
                        VkResultToString(vk::Result(result)));
            return false;
        }
        m_Surface = vk::SurfaceKHR(surface);
        m_Window  = window;

        vk::Bool32 supported = VK_FALSE;
        VkCall(device.Physical().getSurfaceSupportKHR(
            device.GraphicsFamily(), m_Surface, &supported));
        if (!supported)
        {
            VtCoreError(
                "Vulkan: The graphics queue can't present to this window");
            Release();
            return false;
        }
        return true;
    }

    void VulkanSurface::Release()
    {
        VulkanDevice::Get().Defer(DeferredDeletion::Type::eSurface, m_Surface);
        m_Surface = VK_NULL_HANDLE;
    }

    void VulkanSurface::Destroy()
    {
        if (m_Surface) VulkanDevice::Get().Instance().destroySurfaceKHR(m_Surface);
        m_Surface = VK_NULL_HANDLE;
    }

    vk::SurfaceCapabilitiesKHR VulkanSurface::Capabilities() const
    {
        vk::SurfaceCapabilitiesKHR capabilities{};
        VkCall(VulkanDevice::Get().Physical().getSurfaceCapabilitiesKHR(
            m_Surface, &capabilities));
        return capabilities;
    }
    Vector<vk::SurfaceFormatKHR> VulkanSurface::Formats() const
    {
        auto physical = VulkanDevice::Get().Physical();
        u32  count    = 0;
        VkCall(physical.getSurfaceFormatsKHR(m_Surface, &count, nullptr));
        Vector<vk::SurfaceFormatKHR> formats(count);
        VkCall(physical.getSurfaceFormatsKHR(m_Surface, &count, formats.Raw()));
        return formats;
    }
    Vector<vk::PresentModeKHR> VulkanSurface::PresentModes() const
    {
        auto physical = VulkanDevice::Get().Physical();
        u32  count    = 0;
        VkCall(physical.getSurfacePresentModesKHR(m_Surface, &count, nullptr));
        Vector<vk::PresentModeKHR> modes(count);
        VkCall(physical.getSurfacePresentModesKHR(m_Surface, &count, modes.Raw()));
        return modes;
    }
} // namespace Vortex::Vulkan
