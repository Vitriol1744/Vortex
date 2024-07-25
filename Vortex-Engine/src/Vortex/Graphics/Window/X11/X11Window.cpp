/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Graphics/Window/X11/X11Window.hpp"

namespace Vortex
{
    usize       X11Window::s_WindowsCount = 0;

    static void glfwErrorCallback(int code, const char* description)
    {
        (void)code;
        (void)description;
        VtCoreError(
            "GLFW: An error has occured, ErrorCode: {}, Description: {}", code,
            description);
    }

    X11Window::X11Window(const WindowSpecification& specification)
    {
        if (s_WindowsCount == 0)
        {
            VtCoreAssert(X11Window::Initialize());
            VtCoreInfo("GLFW: Successfully initialized, version: {}",
                       glfwGetVersionString());
        }

        i32         width  = specification.VideoMode.Width;
        i32         height = specification.VideoMode.Height;
        const char* title  = specification.Title.data();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, specification.Visible);
        glfwWindowHint(GLFW_DECORATED, specification.Decorated);
        glfwWindowHint(GLFW_FOCUSED, specification.Focused);
        glfwWindowHint(GLFW_FLOATING, specification.AlwaysOnTop);
        glfwWindowHint(GLFW_MAXIMIZED, specification.Maximized);
        glfwWindowHint(GLFW_POSITION_X, specification.Position.x);
        glfwWindowHint(GLFW_POSITION_Y, specification.Position.y);

        Ref<Monitor> monitor = specification.Monitor;
        GLFWmonitor* monitorHandle
            = monitor ? std::any_cast<GLFWmonitor*>(monitor->GetNativeHandle())
                      : nullptr;

        m_Window = glfwCreateWindow(
            width, height, title,
            specification.Fullscreen ? glfwGetPrimaryMonitor() : monitorHandle,
            nullptr);

        if (monitor)
        {
            std::string_view monitorName = monitor->GetName();
            VideoMode        currentMode
                = specification.Monitor->GetCurrentVideoMode();
            u32 bitsPerPixel = currentMode.RedBits + currentMode.GreenBits
                             + currentMode.BlueBits;
            u16 redBits     = currentMode.RedBits;
            u16 greenBits   = currentMode.GreenBits;
            u16 blueBits    = currentMode.BlueBits;
            u32 refreshRate = currentMode.RefreshRate;

            VtCoreTrace(
                "GLFW: Using monitor: {{ name: {}, currentMode: '{} x {} x {} "
                "({} "
                "{} {}) {} Hz'}}",
                monitorName, currentMode.Width, currentMode.Height,
                bitsPerPixel, redBits, greenBits, blueBits, refreshRate);
        }
        VtCoreTrace(
            "GLFW: Created window {{ width: {}, height: {}, title: {} }}",
            width, height, title);

        ++s_WindowsCount;
        SetVisible(true);
        m_Data.IsOpen = !glfwWindowShouldClose(m_Window);

        glfwSetWindowUserPointer(m_Window, reinterpret_cast<void*>(this));
    }
    X11Window::~X11Window()
    {
        glfwDestroyWindow(m_Window);
        --s_WindowsCount;
        if (s_WindowsCount == 0) Shutdown();
    }

    void X11Window::PollEvents() { glfwPollEvents(); }
    void X11Window::Present()
    {
        // TODO(v1tr10l7): Present the framebuffer
    }

    bool X11Window::IsFocused() const noexcept { return m_Data.Focused; }
    bool X11Window::IsMinimized() const noexcept
    {
        return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
    }
    bool X11Window::IsHovered() const noexcept
    {
        return glfwGetWindowAttrib(m_Window, GLFW_HOVERED);
    }
    std::string X11Window::GetTitle() const noexcept
    {
        return glfwGetWindowTitle(m_Window);
    }
    Vec2i X11Window::GetPosition() const noexcept
    {
        Vec2i position;
        glfwGetWindowPos(m_Window, &position.x, &position.y);
        return position;
    }
    inline i32 X11Window::GetWidth() const noexcept
    {
        return m_Data.VideoMode.Width;
    }
    i32 X11Window::GetHeight() const noexcept
    {
        return m_Data.VideoMode.Height;
    }

    void X11Window::Close() noexcept { glfwWindowShouldClose(m_Window); }
    void X11Window::RequestFocus() noexcept { glfwFocusWindow(m_Window); }
    void X11Window::Maximize() noexcept { glfwMaximizeWindow(m_Window); }
    void X11Window::Minimize() noexcept { glfwIconifyWindow(m_Window); }
    void X11Window::SetTitle(std::string_view title)
    {
        glfwSetWindowTitle(m_Window, title.data());
    }
    void X11Window::SetPosition(i32 x, i32 y) const
    {
        glfwSetWindowPos(m_Window, x, y);
    }
    void X11Window::SetWidth(const i32 width) noexcept
    {
        glfwSetWindowSize(m_Window, width, m_Data.VideoMode.Height);
    }
    void X11Window::SetHeight(const i32 height) noexcept
    {
        glfwSetWindowSize(m_Window, m_Data.VideoMode.Width, height);
    }
    void X11Window::ShowCursor() const noexcept
    {
        VtCoreError("ShowCursor: Not implemented");
    }
    void X11Window::HideCursor() const noexcept
    {
        VtCoreError("HideCursor: Not Implemented");
    }
    void X11Window::SetFullscreen(bool fullscreen)
    {
        (void)fullscreen;
        VtCoreError(
            "Vortex currently doesn't support switching fullscreen on this "
            "platform.");
    }
    void X11Window::SetResizable(bool resizable) noexcept
    {
        glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, resizable);
    }
    void X11Window::SetVisible(bool visible) const
    {
        if (visible) return glfwShowWindow(m_Window);
        glfwHideWindow(m_Window);
    }
    void X11Window::SetAlwaysOnTop(bool alwaysOnTop)
    {
        glfwSetWindowAttrib(m_Window, GLFW_FLOATING, alwaysOnTop);
    }

    bool X11Window::Initialize()
    {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, GLFW_FALSE);
        // TODO(v1tr10l7): Custom glfw allocator

        glfwSetErrorCallback(glfwErrorCallback);

        return glfwInit() == GLFW_TRUE;
    }
    void X11Window::Shutdown() { glfwTerminate(); }
}; // namespace Vortex
