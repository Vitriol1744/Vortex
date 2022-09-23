//
// Created by Vitriol1744 on 17.09.2022.
//
#ifndef VORTEX_VORTEXEVENTS_HPP
#define VORTEX_VORTEXEVENTS_HPP

#include "Vortex/Core/Events/IEvent.hpp"

#include "Vortex/Core/Input/KeyCode.hpp"
#include "Vortex/Core/Input/MouseCode.hpp"

#include "Vortex/Core/HashedString.hpp"
#include "Vortex/Core/Types.hpp"

//TODO: Temporary
#include "Vortex/Graphics/Window/IWindow.hpp"

namespace Vortex
{
    VT_REGISTER_EVENT(KeyPressedEvent)
    {
        VT_EVENT_CLASS_ID(KeyPressedEvent)

        IWindow* window;
        Input::KeyCode keycode;
        bool repeated;
    };
    VT_REGISTER_EVENT(KeyReleasedEvent)
    {
        VT_EVENT_CLASS_ID(KeyReleasedEvent);

        IWindow* window;
        Input::KeyCode keycode;
    };
    VT_REGISTER_EVENT(KeyTypedEvent)
    {
        VT_EVENT_CLASS_ID(KeyTypedEvent);

        IWindow* window;
        uint32 character;
    };
    VT_REGISTER_EVENT(MouseButtonPressedEvent)
    {
        VT_EVENT_CLASS_ID(MouseButtonPressedEvent);

        IWindow* window;
        Input::MouseCode mouseCode;
        bool doubleClick;
    };
    VT_REGISTER_EVENT(MouseButtonReleasedEvent)
    {
        VT_EVENT_CLASS_ID(MouseButtonReleasedEvent);

        IWindow* window;
        Input::MouseCode mouseCode;
    };
    VT_REGISTER_EVENT(MouseCursorEnterEvent)
    {
        VT_EVENT_CLASS_ID(MouseCursorEnterEvent);

        IWindow* window;
        bool entered;
    };
    VT_REGISTER_EVENT(MouseScrolledEvent)
    {
        VT_EVENT_CLASS_ID(MouseScrolledEvent);

        IWindow* window;
        Math::Vec2d scroll;
    };
    VT_REGISTER_EVENT(MouseMovedEvent)
    {
        VT_EVENT_CLASS_ID(MouseMovedEvent);

        IWindow* window;
        Math::Vec2d position;
    };
    VT_REGISTER_EVENT(WindowClosedEvent)
    {
        VT_EVENT_CLASS_ID(WindowClosedEvent);

        IWindow* window;
    };
    VT_REGISTER_EVENT(WindowFocusChangedEvent)
    {
        VT_EVENT_CLASS_ID(WindowFocusChangedEvent);

        IWindow* window;
        bool focused;
    };
    VT_REGISTER_EVENT(WindowMovedEvent)
    {
        VT_EVENT_CLASS_ID(WindowMovedEvent);

        IWindow* window;
        Math::Vec2 position;
    };
    VT_REGISTER_EVENT(WindowResizedEvent)
    {
        VT_EVENT_CLASS_ID(WindowResizedEvent);

        IWindow* window;
        Math::Vec2 size;
    };
    VT_REGISTER_EVENT(WindowMinimizedEvent)
    {
        VT_EVENT_CLASS_ID(WindowMinimizedEvent);

        IWindow* window;
    };
    VT_REGISTER_EVENT(WindowMaximizedEvent)
    {
        VT_EVENT_CLASS_ID(WindowMaximizedEvent);

        IWindow* window;
    };
    VT_REGISTER_EVENT(FramebufferResizedEvent)
    {
        VT_EVENT_CLASS_ID(FramebufferResizedEvent);

        IWindow* window;
        Math::Vec2 size;
    };
}

#endif //VORTEX_VORTEXEVENTS_HPP
