/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Platform.hpp"
#include "Vortex/Graphics/Window/Window.hpp"

#ifdef VT_PLATFORM_LINUX
    #include "Vortex/Graphics/Window/X11/X11Window.hpp"
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
        Event<Window*, bool>                   focusChangedEvent;
        Event<Window*, Input::KeyCode, u32>    keyPressedEvent;
        Event<Window*, Input::KeyCode>         keyReleasedEvent;
        Event<Window*, u32>                    keyTypedEvent;
        Event<Window*, Input::MouseCode, bool> mouseButtonPressedEvent;
        Event<Window*, Input::MouseCode>       mouseButtonReleasedEvent;
        Event<Window*, bool>                   mouseCursorEnterEvent;
        Event<Window*, Vec2>                   mouseScrolledEvent;
        Event<Window*, Vec2>                   mouseMovedEvent;
        Event<Window*>                         windowClosedEvent;
        Event<Window*, Vec2>                   windowMovedEvent;
        Event<Window*, Vec2u>                  windowResizedEvent;
    } // namespace WindowEvents

}; // namespace Vortex
