//
// Created by Vitriol1744 on 22.06.2021.
//
#include "vtpch.hpp"
#include "IWindow.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include "Vortex/Graphics/Window/Win32/Win32Window.hpp"
    #undef CreateWindow
#elif defined(VT_PLATFORM_LINUX)
    #include "Vortex/Graphics/Window/Posix/X11Window.hpp"
#endif

namespace Vortex
{
    Ref<IWindow> IWindow::CreateWindow(uint32 width, uint32 height, std::string_view title, Ref<IWindow> share)
    {
        return CreateWindow(width, height, 32, title, share);
    }
    Ref<IWindow> IWindow::CreateWindow(uint32 width, uint32 height, uint32 bitsPerPixel, std::string_view title, Ref<IWindow> share)
    {
        return CreateRef<WindowImpl>(width, height, bitsPerPixel, title, share);
    }

    void IWindow::PollEvents() { WindowImpl::PollEvents(); }

    namespace WindowEvents
    {
        Event<IWindow*, bool>                   focusChangedEvent;
        Event<IWindow*, Input::KeyCode, uint32> keyPressedEvent;
        Event<IWindow*, Input::KeyCode>         keyReleasedEvent;
        Event<IWindow*, uint32>                 keyTypedEvent;
        Event<void*, Math::Vec2, int32>         monitorResolutionChangedEvent;
        Event<void*, MonitorState>              monitorStateChangedEvent;
        Event<IWindow*, Input::MouseCode, bool> mouseButtonPressedEvent;
        Event<IWindow*, Input::MouseCode>       mouseButtonReleasedEvent;
        Event<IWindow*, bool>                   mouseCursorEnterEvent;
        Event<IWindow*, Math::Vec2>             mouseScrolledEvent;
        Event<IWindow*, Math::Vec2>             mouseMovedEvent;
        Event<IWindow*>                         windowClosedEvent;
        Event<IWindow*, Math::Vec2>             windowMovedEvent;
        Event<IWindow*, Math::Vec2u>            windowResizedEvent;
    }
}