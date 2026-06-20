/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/StringUtils.hpp>
#include <Vortex/Core/Platform.hpp>
#include <Vortex/Window/Window.hpp>

#ifdef VT_PLATFORM_LINUX
    #include <Vortex/Window/Wayland/WaylandWindow.hpp>
    #include <Vortex/Window/X11/X11Window.hpp>
#elifdef VT_PLATFORM_WINDOWS
    #include <Vortex/Window/Win32/Win32Window.hpp>
#endif

#ifndef VT_PLATFORM_ANDROID
    #include <nfd.hpp>
#endif

namespace Vortex
{
    static enum WindowSubsystem ChooseSubsystem()
    {
#ifdef VT_PLATFORM_LINUX
        bool        waylandAvailable = ::std::getenv("WAYLAND_DISPLAY");
        bool        xlibAvailable    = ::std::getenv("DISPLAY");

        const char* platformOverride = ::std::getenv("VT_PLATFORM_OVERRIDE");
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
#elifdef VT_PLATFORM_ANDROID
        return WindowSubsystem::eAndroidSurface;
#else
    #error "Platform not supported!"
#endif
    };

    static ::std::function<Window*(const WindowSpecification&)> s_CreateWindow;
    static ::std::function<void()>                              s_PollEvents;
    static ::std::function<void()>                              s_Shutdown;

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

    void Window::Initialize()
    {
#ifdef VT_PLATFORM_LINUX
        if (WindowSubsystem() == WindowSubsystem::eX11)
        {
            X11::Initialize();
            s_CreateWindow = [](const WindowSpecification& specs) -> Window*
            { return new X11Window(specs); };
            s_PollEvents = X11Window::PollEvents;
            s_Shutdown   = X11::Shutdown;
        }
        else if (WindowSubsystem() == WindowSubsystem::eWayland)
        {
            WaylandWindow::Initialize();
            s_CreateWindow = [](const WindowSpecification& specs) -> Window*
            { return new WaylandWindow(specs); };
            s_PollEvents = WaylandWindow::PollEvents;
            s_Shutdown   = Wayland::Shutdown;
        }
#elifdef VT_PLATFORM_WINDOWS
        s_CreateWindow = [](const WindowSpecification& specs) -> Window*
        { return new Win32Window(specs); };
        s_PollEvents = Win32Window::PollEvents;
        s_Shutdown   = Win32Window::Shutdown;
#endif

#ifndef VT_PLATFORM_ANDROID
        NFD_Init();
#endif
        VtCoreAssertMsg(s_CreateWindow && s_PollEvents && s_Shutdown,
                        "Unsupported platform!");
    }
    void Window::Shutdown()
    {
#ifndef VT_PLATFORM_ANDROID
        NFD_Quit();
#endif

        s_Shutdown();
        WindowEvents::MouseButtonPressedEvent.RemoveAllListeners();
        WindowEvents::MouseButtonReleasedEvent.RemoveAllListeners();
        WindowEvents::MouseScrolledEvent.RemoveAllListeners();
        WindowEvents::MouseMovedEvent.RemoveAllListeners();
        WindowEvents::MouseEnteredEvent.RemoveAllListeners();
        WindowEvents::MouseLeftEvent.RemoveAllListeners();
        WindowEvents::WindowMovedEvent.RemoveAllListeners();
        WindowEvents::WindowFocusedEvent.RemoveAllListeners();
        WindowEvents::WindowFocusLostEvent.RemoveAllListeners();
        WindowEvents::WindowMaximizedEvent.RemoveAllListeners();
        WindowEvents::WindowMinimizedEvent.RemoveAllListeners();
        WindowEvents::WindowClosedEvent.RemoveAllListeners();
        WindowEvents::FramebufferResizedEvent.RemoveAllListeners();
        WindowEvents::FileDroppedEvent.RemoveAllListeners();
        WindowEvents::GamepadConnectedEvent.RemoveAllListeners();
        WindowEvents::GamepadDisconnectedEvent.RemoveAllListeners();
        MonitorEvents::MonitorStateChangedEvent.RemoveAllListeners();
    }

    void Window::PollEvents() { s_PollEvents(); }

    Path Window::OpenFileDialog(PathView initialFolder)
    {
        // TODO(v1tr10l7): add filters
#ifndef VT_PLATFORM_ANDROID
        NFD::UniquePath filePath;
        IgnoreUnused(initialFolder);
        nfdresult_t result = NFD::OpenDialog(
            filePath, nullptr, 0,
            !initialFolder.Empty() ? initialFolder.Raw() : nullptr);

        VtCoreAssertFormat(
            result != NFD_ERROR,
            "Window::OpenFileDialog: Failed to open dialog: '{}'",
            NFD::GetError());

        return result == NFD_OKAY ? filePath.get() : "";
#else
        return "";
#endif
    }
    Path Window::SaveFileDialog()
    {
#ifndef VT_PLATFORM_ANDROID
        NFD::UniquePath filePath;
        nfdresult_t     result = NFD::SaveDialog(filePath, nullptr, 0);

        VtCoreAssertFormat(
            result != NFD_ERROR,
            "Window::SaveFileDialog: Failed to open dialog: '{}'",
            NFD::GetError());

        return result == NFD_OKAY ? filePath.get() : "";
#else
        return "";
#endif
    }

    Scope<Window> Window::Create(const WindowSpecification& specs)
    {
        return Scope<Window>(s_CreateWindow(specs));
    }

    Window* Window::CreateForImGui(const WindowSpecification& specs)
    {
        const_cast<WindowSpecification&>(specs).NoAPI = true;

        return s_CreateWindow(specs);
    }

    WindowSubsystem Window::WindowSubsystem()
    {
        static enum WindowSubsystem subsystem = []() -> enum WindowSubsystem
        {
            enum WindowSubsystem subsystem = ChooseSubsystem();

            VtCoreInfo("Window: Using {}...",
                       StringUtils::ToString(subsystem).Raw() + 1);
            return subsystem;
        }
        ();

        return subsystem;
    };

    namespace WindowEvents
    {
        using Input::KeyCode;
        using Input::MouseCode;

        Event<Window*, MouseCode> MouseButtonPressedEvent;
        Event<Window*, MouseCode> MouseButtonReleasedEvent;
        Event<Window*, f64, f64>  MouseScrolledEvent;
        Event<Window*, f64, f64>  MouseMovedEvent;
        Event<Window*>            MouseEnteredEvent;
        Event<Window*>            MouseLeftEvent;

        Event<Window*, i32, i32>  WindowMovedEvent;
        Event<Window*>            WindowFocusedEvent;
        Event<Window*>            WindowFocusLostEvent;
        Event<Window*, bool>      WindowMaximizedEvent;
        Event<Window*, bool>      WindowMinimizedEvent;
        Event<Window*>            WindowClosedEvent;

        Event<Window*, i32, i32>  FramebufferResizedEvent;
        Event<Window*, String>    FileDroppedEvent;
        Event<i32>                GamepadConnectedEvent;
        Event<i32>                GamepadDisconnectedEvent;
    } // namespace WindowEvents

}; // namespace Vortex
