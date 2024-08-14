/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Platform.hpp"
#include "Vortex/Renderer/Window/Window.hpp"

#ifdef VT_PLATFORM_LINUX
    // NOTE(v1tr10l7): if we don't expose both api's, compiler will start
    // complaining about missing glfw native calls to either wayland or glfw
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #define GLFW_EXPOSE_NATIVE_X11
    #include "Vortex/Renderer/Window/Wayland/WaylandWindow.hpp"
    #include "Vortex/Renderer/Window/X11/X11Window.hpp"
#elifdef VT_PLATFORM_WINDOWS
    #include "Vortex/Renderer/Window/Win32/Win32Window.hpp"
#endif

namespace Vortex
{
    static WindowSubsystem ChooseSubsystem()
    {
#ifdef VT_PLATFORM_LINUX
        const char* xdgSessionType = std::getenv("XDG_SESSION_TYPE");
        bool waylandAvailable = (std::strcmp(xdgSessionType, "wayland") == 0)
                             && std::getenv("WAYLAND_DISPLAY");
        bool        xlibAvailable    = std::getenv("DISPLAY");

        const char* platformOverride = std::getenv("VT_PLATFORM_OVERRIDE");
        if (platformOverride)
        {
            if (strcmp(platformOverride, "wayland") == 0 && waylandAvailable)
                return WindowSubsystem::eWayland;
            else if (strcmp(platformOverride, "x11") == 0 && xlibAvailable)
                return WindowSubsystem::eX11;
            else
                VtCoreWarn(
                    "Unrecognized platform override, VT_PLATFORM_OVERRIDE='{}'",
                    platformOverride);
        }
        else if (waylandAvailable) return WindowSubsystem::eWayland;
        else if (xlibAvailable) return WindowSubsystem::eX11;

        VtCoreAssertMsg(false, "no supported window servers detected");
        return WindowSubsystem::eUndefined;
#elifdef VT_PLATFORM_WINDOWS
        return WindowSubsystem::eWin32;
#else
    #error "Platform not supported!"
#endif
    };

    Window::Window(const WindowSpecification& specs)
    {
        m_Data.VideoMode        = specs.VideoMode;
        m_Data.Title            = specs.Title;
        m_Data.Position         = specs.Position;
        m_Data.IsOpen           = true;
        m_Data.Fullscreen       = specs.Fullscreen;
        m_Data.Resizable        = specs.Resizable;
        m_Data.Visible          = specs.Visible;
        m_Data.Focused          = specs.Focused;
        m_Data.AlwaysOnTop      = specs.AlwaysOnTop;
        m_Data.MouseHovered     = false;
        m_Data.Decorated        = specs.Decorated;
        m_Data.FocusOnShow      = specs.FocusOnShow;
        m_Data.AutoIconify      = specs.AutoIconify;
        m_Data.MousePassthrough = false;
    }

    void Window::PollEvents()
    {
#ifdef VT_PLATFORM_LINUX
        if (GetWindowSubsystem() == WindowSubsystem::eX11)
            X11Window::PollEvents();
        else if (GetWindowSubsystem() == WindowSubsystem::eWayland)
            WaylandWindow::PollEvents();
        else VtCoreFatal("This platform is not supported by Vortex!");
#elifdef VT_PLATFORM_WINDOWS
        Win32Window::PollEvents();
#else
    #error "Polling events is not supported by Vortex on this platform."
#endif
    }

    Ref<Window> Window::Create(const WindowSpecification& specs)
    {
        Ref<Window> ret = nullptr;
#ifdef VT_PLATFORM_LINUX
        WindowSubsystem subsystem = GetWindowSubsystem();
        if (subsystem == WindowSubsystem::eX11)
            ret = CreateRef<X11Window>(specs);
        else if (subsystem == WindowSubsystem::eWayland)
            ret = CreateRef<WaylandWindow>(specs);
#elifdef VT_PLATFORM_WINDOWS
        ret = CreateRef<Win32Window>(specs);
#endif

        return ret;
    }

    Window* Window::CreateForImGui(const WindowSpecification& specs)
    {
        const_cast<WindowSpecification&>(specs).NoAPI = true;

        Window* ret                                   = nullptr;
#ifdef VT_PLATFORM_LINUX
        WindowSubsystem subsystem = GetWindowSubsystem();
        if (subsystem == WindowSubsystem::eX11) ret = new X11Window(specs);
        else if (subsystem == WindowSubsystem::eWayland)
            ret = new WaylandWindow(specs);
#elifdef VT_PLATFORM_WINDOWS
        ret = new Win32Window(specs);
#endif

        return ret;
    }

    WindowSubsystem Window::GetWindowSubsystem()
    {
        static WindowSubsystem subsystem = []() -> WindowSubsystem
        {
            WindowSubsystem subsystem = ChooseSubsystem();

            VtCoreInfo("Window: Using {}...",
                       magic_enum::enum_name(subsystem).data() + 1);
            return subsystem;
        }();

        return subsystem;
    };

    namespace WindowEvents
    {
        using Input::KeyCode;
        using Input::MouseCode;

        Event<Window*, KeyCode, u32> KeyPressedEvent;
        Event<Window*, KeyCode>      KeyReleasedEvent;
        Event<Window*, u32>          KeyTypedEvent;
        Event<Window*, u32, KeyCode> UnicodeKeyTypedEvent;

        Event<Window*, MouseCode>    MouseButtonPressedEvent;
        Event<Window*, MouseCode>    MouseButtonReleasedEvent;
        Event<Window*, f64, f64>     MouseScrolledEvent;
        Event<Window*, f64, f64>     MouseMovedEvent;
        Event<Window*>               MouseEnteredEvent;
        Event<Window*>               MouseLeftEvent;

        Event<Window*, i32, i32>     WindowMovedEvent;
        Event<Window*, i32, i32>     WindowResizedEvent;
        Event<Window*>               WindowFocusedEvent;
        Event<Window*>               WindowFocusLostEvent;
        Event<Window*, bool>         WindowMaximizedEvent;
        Event<Window*, bool>         WindowMinimizedEvent;
        Event<Window*>               WindowClosedEvent;

        Event<Window*, i32, i32>     FramebufferResizedEvent;
        Event<Window*, std::string>  FileDroppedEvent;
        Event<i32>                   GamepadConnectedEvent;
        Event<i32>                   GamepadDisconnectedEvent;
    } // namespace WindowEvents

}; // namespace Vortex
