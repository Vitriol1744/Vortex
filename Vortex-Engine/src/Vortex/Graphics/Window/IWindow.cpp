/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Platform.hpp"
#include "Vortex/Graphics/Window/IWindow.hpp"

#ifdef VT_PLATFORM_LINUX
    #include "Vortex/Graphics/Window/X11/X11Window.hpp"
#endif

namespace Vortex
{
    void IWindow::PollEvents()
    {
#ifdef VT_PLATFORM_LINUX
        X11Window::PollEvents();
#else
    #error "Polling events is not supported by Vortex on this platform."
#endif
    }

    Ref<IWindow> IWindow::Create(const WindowSpecification& specification)
    {
#ifdef VT_PLATFORM_LINUX
        return CreateRef<X11Window>(specification);
#else
    #error "Window creation is not supported by Vortex on this platform."
#endif
    }

    namespace WindowEvents
    {
        Event<IWindow*, bool>                   focusChangedEvent;
        Event<IWindow*, Input::KeyCode, u32>    keyPressedEvent;
        Event<IWindow*, Input::KeyCode>         keyReleasedEvent;
        Event<IWindow*, u32>                    keyTypedEvent;
        Event<void*, Vec2, i32>                 monitorResolutionChangedEvent;
        Event<void*, MonitorState>              monitorStateChangedEvent;
        Event<IWindow*, Input::MouseCode, bool> mouseButtonPressedEvent;
        Event<IWindow*, Input::MouseCode>       mouseButtonReleasedEvent;
        Event<IWindow*, bool>                   mouseCursorEnterEvent;
        Event<IWindow*, Vec2>                   mouseScrolledEvent;
        Event<IWindow*, Vec2>                   mouseMovedEvent;
        Event<IWindow*>                         windowClosedEvent;
        Event<IWindow*, Vec2>                   windowMovedEvent;
        Event<IWindow*, Vec2u>                  windowResizedEvent;
    } // namespace WindowEvents
};    // namespace Vortex
