/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Renderer/Window/X11/X11Window.hpp"

#include <X11/Xresource.h>
#include <xcb/xproto.h>

namespace Vortex
{
    usize X11Window::s_WindowsCount = 0;
    u32   errorCode                 = 0;

    namespace
    {
        ::Cursor s_BlankCursor = 0;
        i32      XlibErrorHandler(Display* display, XErrorEvent* event)
        {
            if (display == X11::GetDisplay()) errorCode = event->error_code;

            return 0;
        }
    } // namespace

    using Input::KeyCode;
    using Input::MouseCode;

    X11Window::X11Window(const WindowSpecification& specification)
        : Window(specification)
    {
        if (s_WindowsCount == 0)
        {
            X11::Initialize();
            VtCoreInfo("X11: Successfully initialized");
        }

        i32         width  = specification.VideoMode.Width;
        i32         height = specification.VideoMode.Height;
        const char* title  = specification.Title.data();

        m_Data.Visible     = specification.Visible;
        m_Data.Decorated   = specification.Decorated;
        m_Data.Focused     = specification.Focused;
        m_Data.Position    = specification.Position;
        m_Data.AutoIconify = specification.AutoIconify;
        m_Data.FocusOnShow = specification.FocusOnShow;

        long eventMask     = StructureNotifyMask | KeyPressMask | KeyReleaseMask
                       | PointerMotionMask | ButtonPressMask | ButtonReleaseMask
                       | ExposureMask | FocusChangeMask | VisibilityChangeMask
                       | EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

        Ref<Monitor> monitor = specification.Monitor;

        i32          x       = specification.Position.x;
        i32          y       = specification.Position.y;

        Visual*      visual  = DefaultVisual(X11::GetDisplay(),
                                             DefaultScreen(X11::GetDisplay()));
        int          depth
            = DefaultDepth(X11::GetDisplay(), DefaultScreen(X11::GetDisplay()));
        XSetWindowAttributes attributes{};
        attributes.colormap = XCreateColormap(
            X11::GetDisplay(), X11::GetRootWindow(), visual, AllocNone);
        attributes.event_mask        = eventMask;
        attributes.override_redirect = false;

        m_WindowHandle               = XCreateWindow(
            X11::GetDisplay(), X11::GetRootWindow(), x, y, width, height, 0,
            depth, InputOutput, visual,
            CWEventMask | CWBorderPixel | CWColormap, &attributes);
        m_InputContext
            = XCreateIC(X11::GetInputMethod(), XNClientWindow, m_WindowHandle,
                        XNFocusWindow, m_WindowHandle, XNInputStyle,
                        XIMPreeditNothing | XIMStatusNothing, nullptr);
        m_Parent                       = X11::GetRootWindow();
        GetWindowMap()[m_WindowHandle] = this;

        VtCoreTrace(
            "X11: Created window {{ width: {}, height: {}, title: {} }}", width,
            height, title);

        ++s_WindowsCount;
        SetVisible(specification.Visible);
        m_Data.IsOpen = true;

        if (!specification.NoAPI)
            m_RendererContext
                = RendererContext::Create(this, specification.VSync);

        if (s_WindowsCount == 1)
        {
            // Create Blank Cursor
            char   data[1]           = {0};
            Pixmap blankCursorPixMap = XCreateBitmapFromData(
                X11::GetDisplay(), DefaultRootWindow(X11::GetDisplay()), data,
                1, 1);
            XColor blankCursorColor;
            s_BlankCursor = XCreatePixmapCursor(
                X11::GetDisplay(), blankCursorPixMap, blankCursorPixMap,
                &blankCursorColor, &blankCursorColor, 0, 0);

            XFreePixmap(X11::GetDisplay(), blankCursorPixMap);
        }
    }
    X11Window::~X11Window()
    {
        m_RendererContext.reset();
        GetWindowMap().erase(m_WindowHandle);
        XDestroyWindow(X11::GetDisplay(), m_WindowHandle);
        --s_WindowsCount;
        if (s_WindowsCount == 0) X11::Shutdown();
    }

    void X11Window::PollEvents()
    {
        Display* display = X11::GetDisplay();
        XPending(display);

        while (QLength(X11::GetDisplay()))
        {
            XEvent event;
            XNextEvent(X11::GetDisplay(), &event);
            ProcessEvent(event);
        }

        XFlush(display);
    }
    void X11Window::Present() { m_RendererContext->Present(); }

    bool X11Window::IsFocused() const noexcept { return m_Data.Focused; }
    bool X11Window::IsMinimized() const noexcept
    {
        VtTodo();
        return false;
    }
    bool X11Window::IsHovered() const noexcept
    {
        auto releaseHandler = [](auto oldHandler)
        {
            XSync(X11::GetDisplay(), False);
            XSetErrorHandler(oldHandler);
        };

        ::Window w = X11::GetRootWindow();
        while (w)
        {
            ::Window   root;
            i32        rootX, rootY, childX, childY;
            u32        mask;

            auto       oldHandler = XSetErrorHandler(XlibErrorHandler);

            const Bool result
                = XQueryPointer(X11::GetDisplay(), w, &root, &w, &rootX, &rootY,
                                &childX, &childY, &mask);
            releaseHandler(oldHandler);

            if (errorCode == BadWindow) w = X11::GetRootWindow();
            else if (!result) return false;
            else if (w == m_WindowHandle) return true;
        }

        return false;
    }
    std::string X11Window::GetTitle() const noexcept { return m_Data.Title; }
    Vec2i       X11Window::GetPosition() const noexcept
    {
        Vec2i    position;
        ::Window dummy;
        XTranslateCoordinates(X11::GetDisplay(), m_WindowHandle,
                              X11::GetRootWindow(), 0, 0, &position.x,
                              &position.y, &dummy);

        return position;
    }
    inline Vec2i X11Window::GetSize() const noexcept
    {
        XWindowAttributes attributes{};
        XGetWindowAttributes(X11::GetDisplay(), m_WindowHandle, &attributes);

        return Vec2i(attributes.width, attributes.height);
    }
    Vec2i X11Window::GetFramebufferSize() const noexcept { return GetSize(); }
    Vec2f X11Window::GetContentScale() const noexcept
    {
        VtTodo();
        return {1.0f, 1.0f};
    }
    f32 X11Window::GetOpacity() const noexcept
    {
        f32 opacity = 1.0f;

        VtTodo();
        return opacity;
    }
    Vec2d X11Window::GetCursorPosition() const noexcept
    {
        ::Window root, child;
        i32      rootX, rootY, childX, childY;
        u32      mask;

        XQueryPointer(X11::GetDisplay(), m_WindowHandle, &root, &child, &rootX,
                      &rootY, &childX, &childY, &mask);
        return Vec2d(static_cast<f64>(childX), static_cast<f64>(childY));
    }

    void X11Window::Close() noexcept { m_Data.IsOpen = false; }
    void X11Window::RequestFocus() noexcept
    {
        Atom netActiveWindow = X11::GetAtom("_NET_ACTIVE_WINDOW", true);

        if (netActiveWindow)
        {
            XEvent event{};
            event.type                 = ClientMessage;
            event.xclient.window       = m_WindowHandle;
            event.xclient.message_type = netActiveWindow;
            event.xclient.format       = 32;
            event.xclient.data.l[0]    = 1;
            event.xclient.data.l[1]    = 0; // lastUserActivityTime;
            event.xclient.data.l[2]    = 0;

            int mask = SubstructureNotifyMask | SubstructureRedirectMask;
            VtCoreAssert(XSendEvent(X11::GetDisplay(),
                                    DefaultRootWindow(X11::GetDisplay()), false,
                                    mask, &event));
        }
        else
        {
            XSetInputFocus(X11::GetDisplay(), m_WindowHandle,
                           RevertToPointerRoot, CurrentTime);
            XRaiseWindow(X11::GetDisplay(), m_WindowHandle);
        }
    }
    void X11Window::RequestUserAttention() const noexcept { VtTodo(); }
    void X11Window::Maximize() noexcept { VtTodo(); }
    void X11Window::Minimize() noexcept { VtTodo(); }
    void X11Window::Restore() noexcept { VtTodo(); }
    void X11Window::SetTitle(std::string_view title)
    {
        VtUnused(title);
        VtTodo();
    }
    void X11Window::SetIcon(const Icon* icons, usize count)
    {
        VtUnused(icons);
        VtUnused(count);
        VtTodo();
    }
    void X11Window::SetPosition(i32 x, i32 y)
    {
        XMoveWindow(X11::GetDisplay(), m_WindowHandle, x, y);
        m_Data.Position.x = x;
        m_Data.Position.y = y;
    }
    void X11Window::SetAspectRatio(i32 numerator, i32 denominator)
    {
        VtUnused(numerator);
        VtUnused(denominator);
        VtTodo();
    }
    void X11Window::SetSize(const Vec2i& size) noexcept
    {
        VtUnused(size);
        VtTodo();
    }
    void X11Window::SetOpacity(f32 opacity)
    {
        const CARD32 value
            = static_cast<CARD32>(0xffffffffu * static_cast<f64>(opacity));
        XChangeProperty(X11::GetDisplay(), m_WindowHandle,
                        X11::GetAtom("_NET_WM_WINDOW_OPACITY"), XA_CARDINAL, 32,
                        PropModeReplace, reinterpret_cast<const u8*>(&value),
                        1);
    }

    void X11Window::SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                 i32 maxHeight)
    {
        VtUnused(minWidth);
        VtUnused(minHeight);
        VtUnused(maxWidth);
        VtUnused(maxHeight);
        VtTodo();
    }

    void X11Window::SetAutoIconify(bool autoIconify) const noexcept
    {
        VtUnused(autoIconify);
        VtTodo();
    }
    void X11Window::SetCursorPosition(Vec2d position) noexcept
    {
        m_WarpCursorPos.x = static_cast<i32>(position.x);
        m_WarpCursorPos.y = static_cast<i32>(position.y);

        XWarpPointer(X11::GetDisplay(), None, m_WindowHandle, 0, 0, 0, 0,
                     m_WarpCursorPos.x, m_WarpCursorPos.y);
        XFlush(X11::GetDisplay());
    }
    void X11Window::ShowCursor() const noexcept
    {
        XDefineCursor(X11::GetDisplay(), m_WindowHandle, None);
    }
    void X11Window::HideCursor() const noexcept
    {
        XDefineCursor(X11::GetDisplay(), m_WindowHandle, s_BlankCursor);
    }
    void X11Window::SetFullscreen(bool fullscreen)
    {
        VtUnused(fullscreen);
        VtTodo();
    }
    void X11Window::SetResizable(bool resizable) noexcept
    {
        VtUnused(resizable);
        VtTodo();
    }
    void X11Window::SetVisible(bool visible) const
    {
        if (visible) XMapWindow(X11::GetDisplay(), m_WindowHandle);
        VtTodo();
    }
    void X11Window::SetAlwaysOnTop(bool alwaysOnTop)
    {
        VtUnused(alwaysOnTop);
        VtTodo();
    }

    void X11Window::ProcessEvent(XEvent& event)
    {
        switch (event.type)
        {
            case ButtonPress:
            {
                auto window = GetWindowMap()[event.xbutton.window];
                if (!window) return;
                switch (event.xbutton.button)
                {
                    case Button1:
                        WindowEvents::MouseButtonPressedEvent(window,
                                                              MouseCode::eLeft);
                        break;
                    case Button2:
                        WindowEvents::MouseButtonPressedEvent(
                            window, MouseCode::eMiddle);
                        break;
                    case Button3:
                        WindowEvents::MouseButtonPressedEvent(
                            window, MouseCode::eRight);
                        break;
                    case Button4:
                        WindowEvents::MouseScrolledEvent(window, 1, 0);
                        break;
                    case Button5:
                        WindowEvents::MouseScrolledEvent(window, -1, 0);
                        break;
                    case Button5 + 1:
                        WindowEvents::MouseScrolledEvent(window, 0, 1);
                        break;
                    case Button5 + 2:
                        WindowEvents::MouseScrolledEvent(window, 0, -1);
                        break;
                        // TODO(v1tr10l7): handle extra mouse buttons
                }
                break;
            }
            case ButtonRelease:
            {
                auto window = GetWindowMap()[event.xbutton.window];
                if (!window) return;
                MouseCode button = MouseCode::eUnknown;
                switch (event.xbutton.button)
                {
                    case Button1: button = MouseCode::eLeft; break;
                    case Button2: button = MouseCode::eMiddle; break;
                    case Button3: button = MouseCode::eRight; break;
                }

                WindowEvents::MouseButtonReleasedEvent(window, button);
                break;
            }
            case ClientMessage:
            {
                auto window = GetWindowMap()[event.xbutton.window];
                if (!window) return;
                static Atom wmProtocols = X11::GetAtom("WM_PROTOCOLS");
                if (event.xclient.message_type == wmProtocols)
                {
                    static Atom wmDeleteWindow
                        = X11::GetAtom("WM_DELETE_WINDOW");
                    [[maybe_unused]] static Atom wmPing
                        = X11::GetAtom("_NET_WM_PING", true);

                    if (event.xclient.format == 32
                        && event.xclient.data.l[0]
                               == static_cast<long>(wmDeleteWindow))
                    {
                        window->m_Data.IsOpen = false;
                        WindowEvents::WindowClosedEvent(window);
                    }
                }

                break;
            }
            case ConfigureNotify:
            {
                auto window = GetWindowMap()[event.xbutton.window];
                if (!window) return;

                i32 width  = event.xconfigure.width;
                i32 height = event.xconfigure.height;
                if (window->m_Data.VideoMode.Width != width
                    || window->m_Data.VideoMode.Height != height)
                {
                    window->m_Data.VideoMode.Width  = width;
                    window->m_Data.VideoMode.Height = height;
                    if (window->m_RendererContext)
                        window->m_RendererContext->OnResize();

                    WindowEvents::FramebufferResizedEvent(window, width,
                                                          height);
                    WindowEvents::WindowResizedEvent(window, width, height);
                }

                i32      xpos       = event.xconfigure.x;
                i32      ypos       = event.xconfigure.y;
                ::Window rootWindow = X11::GetRootWindow();
                if (!event.xany.send_event && window->m_Parent != rootWindow)
                {
                    ::Window dummy;
                    XTranslateCoordinates(X11::GetDisplay(), window->m_Parent,
                                          rootWindow, xpos, ypos, &xpos, &ypos,
                                          &dummy);
                }
                if (window->m_Data.Position.x != xpos
                    || window->m_Data.Position.y != ypos)
                {
                    window->m_Data.Position.x = xpos;
                    window->m_Data.Position.y = ypos;
                    WindowEvents::WindowMovedEvent(window, xpos, ypos);
                }
                break;
            }
            case EnterNotify:
            {
                auto window = GetWindowMap()[event.xcrossing.window];
                if (!window) return;
                const i32 x            = event.xcrossing.x;
                const i32 y            = event.xcrossing.y;

                window->m_MouseHovered = true;
                WindowEvents::MouseEnteredEvent(window);
                WindowEvents::MouseMovedEvent(window, x, y);
                window->m_LastCursorPos.x = window->m_Data.MousePosition.x = x;
                window->m_LastCursorPos.y = window->m_Data.MousePosition.y = y;
                break;
            }
            case FocusIn:
            {
                auto window = GetWindowMap()[event.xfocus.window];
                if (!window) return;
                window->m_Data.Focused = true;
                WindowEvents::WindowFocusedEvent(window);
                break;
            }
            case FocusOut:
            {
                auto window = GetWindowMap()[event.xfocus.window];
                if (!window) return;
                window->m_Data.Focused = false;
                WindowEvents::WindowFocusLostEvent(window);
                break;
            }
            case LeaveNotify:
            {
                auto window = GetWindowMap()[event.xcrossing.window];
                if (!window) return;
                window->m_MouseHovered = false;
                WindowEvents::MouseLeftEvent(window);
                break;
            }
            case MotionNotify:
            {
                auto window = GetWindowMap()[event.xmotion.window];
                if (!window) return;

                const i32 x = event.xmotion.x;
                const i32 y = event.xmotion.y;
                if (x != window->m_WarpCursorPos.x
                    && y != window->m_WarpCursorPos.y)
                {
                    WindowEvents::MouseMovedEvent(window, event.xmotion.x,
                                                  event.xmotion.y);
                }
                window->m_Data.MousePosition.x = window->m_LastCursorPos.x = x;
                window->m_Data.MousePosition.y = window->m_LastCursorPos.y = y;
                break;
            }
            case ReparentNotify:
            {
            }
            default: break;
        }
    }
}; // namespace Vortex
