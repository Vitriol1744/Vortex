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
    #include "Vortex/Renderer/Window/X11/X11Window.hpp"
#endif

namespace Vortex
{
    static WindowSubsystem ChooseSubsystem()
    {
#ifdef VT_PLATFORM_LINUX
        return WindowSubsystem::eX11;
#else
    #error "Platform not supported!"
#endif

        VtCoreFatal("Platform not supported!");
        VtUnreachable();
    };

    void Window::PollEvents()
    {
#ifdef VT_PLATFORM_LINUX
        if (GetWindowSubsystem() == WindowSubsystem::eX11)
            X11Window::PollEvents();
        else
            VtCoreFatal(
                "Only X11 Windowing system is currently supported by Vortex");
#else
    #error "Polling events is not supported by Vortex on this platform."
#endif
    }

    Ref<Window> Window::Create(const WindowSpecification& specification)
    {
        Ref<Window> ret = nullptr;
#ifdef VT_PLATFORM_LINUX
        WindowSubsystem subsystem = GetWindowSubsystem();
        if (subsystem == WindowSubsystem::eX11)
            ret = CreateRef<X11Window>(specification);
#endif

        return ret;
    }

    WindowSubsystem Window::GetWindowSubsystem()
    {
        static WindowSubsystem subsystem = ChooseSubsystem();

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
        Event<Window*>               MouseEnteredEvent;
        Event<Window*>               MouseLeavedEvent;
        Event<Window*, f64, f64>     MouseScrolledEvent;
        Event<Window*, f64, f64>     MouseMovedEvent;
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