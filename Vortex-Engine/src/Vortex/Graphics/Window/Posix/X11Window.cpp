//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Vortex/Platform/Platform.hpp"

#include "Vortex/Graphics/API/OpenGL46/GL46Context.hpp"
#include "Vortex/Graphics/API/IRendererAPI.hpp"

#ifdef VT_PLATFORM_LINUX
#include "X11Window.hpp"

#include "Vortex/Platform/X11/X11.hpp"

#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>

#undef None

static constexpr const unsigned long functionHint           = 1 << 0;
static constexpr const unsigned long decorationsHint        = 1 << 1;

static constexpr const unsigned long borderStyle            = 1 << 1;
static constexpr const unsigned long resizeStyle            = 1 << 2;
static constexpr const unsigned long titleStyle             = 1 << 3;
static constexpr const unsigned long menuStyle              = 1 << 4;
static constexpr const unsigned long minimizeStyle          = 1 << 5;
static constexpr const unsigned long maximizeStyle          = 1 << 6;

static constexpr const unsigned long resizeFunc             = 1 << 1;
static constexpr const unsigned long moveFunc               = 1 << 2;
static constexpr const unsigned long minimizeFunc           = 1 << 3;
static constexpr const unsigned long maximizeFunc           = 1 << 4;
static constexpr const unsigned long closeFunc              = 1 << 5;

namespace Vortex
{
    using namespace Input;
    using namespace Graphics;
    using namespace Math;
    
    namespace
    {
        bool ewmhSupported = false;

        #pragma clang diagnostic push
        #pragma ide diagnostic ignored "OCDFAInspection"
        Input::KeyCode TranslateKeyCode(uint32 keycode)
        {
            switch (keycode)
            {
                case XK_0:              return KeyCode::Num0;
                case XK_1:              return KeyCode::Num1;
                case XK_2:              return KeyCode::Num2;
                case XK_3:              return KeyCode::Num3;
                case XK_4:              return KeyCode::Num4;
                case XK_5:              return KeyCode::Num5;
                case XK_6:              return KeyCode::Num6;
                case XK_7:              return KeyCode::Num7;
                case XK_8:              return KeyCode::Num8;
                case XK_9:              return KeyCode::Num9;
                case XK_a:              return KeyCode::A;
                case XK_b:              return KeyCode::B;
                case XK_c:              return KeyCode::C;
                case XK_d:              return KeyCode::D;
                case XK_e:              return KeyCode::E;
                case XK_f:              return KeyCode::F;
                case XK_g:              return KeyCode::G;
                case XK_h:              return KeyCode::H;
                case XK_i:              return KeyCode::I;
                case XK_j:              return KeyCode::J;
                case XK_k:              return KeyCode::K;
                case XK_l:              return KeyCode::L;
                case XK_m:              return KeyCode::M;
                case XK_n:              return KeyCode::N;
                case XK_o:              return KeyCode::O;
                case XK_p:              return KeyCode::P;
                case XK_q:              return KeyCode::Q;
                case XK_r:              return KeyCode::R;
                case XK_s:              return KeyCode::S;
                case XK_t:              return KeyCode::T;
                case XK_u:              return KeyCode::U;
                case XK_v:              return KeyCode::V;
                case XK_w:              return KeyCode::W;
                case XK_x:              return KeyCode::X;
                case XK_y:              return KeyCode::Y;
                case XK_z:              return KeyCode::Z;
                case XK_grave:          return KeyCode::Tilde;
                case XK_F1:             return KeyCode::F1;
                case XK_F2:             return KeyCode::F2;
                case XK_F3:             return KeyCode::F3;
                case XK_F4:             return KeyCode::F4;
                case XK_F5:             return KeyCode::F5;
                case XK_F6:             return KeyCode::F6;
                case XK_F7:             return KeyCode::F7;
                case XK_F8:             return KeyCode::F8;
                case XK_F9:             return KeyCode::F9;
                case XK_F10:            return KeyCode::F10;
                case XK_F11:            return KeyCode::F11;
                case XK_F12:            return KeyCode::F12;
                case XK_F13:            return KeyCode::F13;
                case XK_F14:            return KeyCode::F14;
                case XK_F15:            return KeyCode::F15;
                case XK_Escape:         return KeyCode::Escape;
                case XK_BackSpace:      return KeyCode::BackSpace;
                case XK_Tab:            return KeyCode::Tab;
                case XK_Caps_Lock:      return KeyCode::CapsLock;
                case XK_KP_Enter:       return KeyCode::Enter;
                case XK_Return:         return KeyCode::Return;
                case XK_Shift_L:        return KeyCode::LShift;
                case XK_Shift_R:        return KeyCode::RShift;
                case XK_Control_L:      return KeyCode::LCtrl;
                case XK_Control_R:      return KeyCode::RCtrl;
                case XK_Alt_L:          return KeyCode::LAlt;
                case XK_Alt_R:          return KeyCode::RAlt;
                case XK_Super_L:        return KeyCode::LSystem;
                case XK_Super_R:        return KeyCode::RSystem;
                case XK_space:          return KeyCode::Space;
                case XK_minus:          return KeyCode::Hyphen;
                case XK_equal:          return KeyCode::Equal;
                case XK_KP_Decimal:     return KeyCode::Decimal;
                case XK_bracketleft:    return KeyCode::LBracket;
                case XK_bracketright:   return KeyCode::RBracket;
                case XK_semicolon:      return KeyCode::Semicolon;
                case XK_apostrophe:     return KeyCode::Apostrophe;
                case XK_comma:          return KeyCode::Comma;
                case XK_period:         return KeyCode::Period;
                case XK_slash:          return KeyCode::Slash;
                case XK_backslash:      return KeyCode::BackSlash;
                case XK_Up:             return KeyCode::Up;
                case XK_Down:           return KeyCode::Down;
                case XK_Left:           return KeyCode::Left;
                case XK_Right:          return KeyCode::Right;
                case XK_KP_Insert:      return KeyCode::Numpad0;
                case XK_KP_End:         return KeyCode::Numpad1;
                case XK_KP_Down:        return KeyCode::Numpad2;
                case XK_KP_Page_Down:   return KeyCode::Numpad3;
                case XK_KP_Left:        return KeyCode::Numpad4;
                case XK_KP_Begin:       return KeyCode::Numpad5;
                case XK_KP_Right:       return KeyCode::Numpad6;
                case XK_KP_Home:        return KeyCode::Numpad7;
                case XK_KP_Up:          return KeyCode::Numpad8;
                case XK_KP_Page_Up:     return KeyCode::Numpad9;
                case XK_KP_Separator:   return KeyCode::Separator;
                case XK_KP_Add:         return KeyCode::Add;
                case XK_KP_Subtract:    return KeyCode::Subtract;
                case XK_KP_Multiply:    return KeyCode::Multiply;
                case XK_KP_Divide:      return KeyCode::Divide;
                case XK_Insert:         return KeyCode::Insert;
                case XK_Delete:         return KeyCode::Delete;
                case XK_Page_Up:        return KeyCode::PageUp;
                case XK_Page_Down:      return KeyCode::PageDown;
                case XK_Home:           return KeyCode::Home;
                case XK_End:            return KeyCode::End;

                default:                return KeyCode::Unknown;
            }
        }
        #pragma clang diagnostic pop

        static Bool IsEwmhSupported(Display* display)
        {
            Bool result = False;

            Atom wmCheck = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", true);
            Atom netSupported = XInternAtom(display, "_NET_SUPPORTED", true) & wmCheck;
            if (!netSupported) return false;

            int status;
            Atom type;
            int format;
            unsigned long items;
            unsigned long bytes;
            unsigned char* data;
            Window rootWindow = DefaultRootWindow(display);

            status = XGetWindowProperty(display, rootWindow, wmCheck, 0, 1, 0, 33, &type, &format, &items, &bytes, &data);

            if (status != Success || type != XA_WINDOW || items != 1)
            {
                if (status == Success) XFree(data);

                return false;
            }
            rootWindow = *reinterpret_cast<Window*>(data);
            XFree(data);
            if (!rootWindow) return false;

            status = XGetWindowProperty(display, rootWindow, wmCheck, 0, 1, 0, 33, &type, &format, &items, &bytes, &data);

            if (status != Success || type != XA_WINDOW || items != 1)
            {
                if (status == Success) XFree(data);

                return false;
            }

            Window childWindow = *reinterpret_cast<Window*>(data);
            XFree(data);
            if (!childWindow) return false;

            if (rootWindow != childWindow) return False;
            result = True;

            if (result == Success) XFree(data);

            return result;
        }
    }

    uint32      WindowImpl::windowsCount    = 0;
    Display*    WindowImpl::display         = nullptr;
    XIM         WindowImpl::inputMethod     = nullptr;
    Cursor      WindowImpl::blankCursor     = 0;

    WindowImpl::WindowImpl(int32 width, int32 height, uint32 bitsPerPixel, std::string_view title, Ref<IWindow> share)
    {
        // Initialize Variables
        if (windowsCount == 0) Initialize();
        memset(keys, 0, static_cast<uint32>(KeyCode::KeysCount));
        memset(buttons, 0, static_cast<uint32>(MouseCode::ButtonsCount));
        data.width = width;
        data.height = height;
        int32 x = data.position.x = 0;
        int32 y = data.position.y = 0;
        lastUserActivityTime = 0;
        rootWindow = DefaultRootWindow(display);
        screen = DefaultScreenOfDisplay(display);
        data.sharedContext = share ? share->GetGraphicsContext() : nullptr;

        // Create Window
        Create(width, height, bitsPerPixel, title, data.sharedContext);

        if (windowsCount == 1)
        {
            // Create Blank Cursor
            char data[1] = {0};
            Pixmap blankCursorPixMap = XCreateBitmapFromData(display, DefaultRootWindow(display), data, 1, 1);
            XColor blankCursorColor;
            blankCursor = XCreatePixmapCursor(display, blankCursorPixMap, blankCursorPixMap, &blankCursorColor, &blankCursorColor, 0, 0);

            XFreePixmap(display, blankCursorPixMap);
        }
    }
    WindowImpl::~WindowImpl()
    {
        Destroy();
        if (windowsCount == 0) Shutdown();
    }

    void WindowImpl::PollEvents()
    {
        while (XEventsQueued(display, QueuedAlready))
        {
            XEvent event;
            XNextEvent(display, &event);

            HandleEvent(event);
        }
    }
    void WindowImpl::Present()
    {
        data.graphicsContext->Present();
    }

    void WindowImpl::ShowCursor() const noexcept
    {
        XDefineCursor(display, window, XNone);
    }
    void WindowImpl::HideCursor() const noexcept
    {
        XDefineCursor(display, window, blankCursor);
    }
    void WindowImpl::RequestFocus() const
    {
        Atom netActiveWindow = XInternAtom(display, "_NET_ACTIVE_WINDOW", true);

        if (netActiveWindow)
        {
            XEvent event;
            memset(&event, 0, sizeof(event));

            event.type = ClientMessage;
            event.xclient.window = window;
            event.xclient.message_type = netActiveWindow;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 1;
            event.xclient.data.l[1] = lastUserActivityTime;
            event.xclient.data.l[2] = 0;

            int mask = SubstructureNotifyMask | SubstructureRedirectMask;
            VT_CORE_ASSERT(XSendEvent(display, DefaultRootWindow(display), false, mask, &event));
        }
        else
        {
            XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);
            XRaiseWindow(display, window);
        }
    }
    void WindowImpl::SetFullscreen(bool fullscreen)
    {
        if (fullscreen != data.fullscreen)
        {
            if (fullscreen)
            {
                Atom wmBypassCompositor = XInternAtom(display, "_NET_WM_BYPASS_COMPOSITOR", true);
                if (wmBypassCompositor)
                {
                    static const unsigned char bypassCompositor = 1;
                    XChangeProperty(display, window, bypassCompositor, XA_CARDINAL, 32, PropModeReplace, &bypassCompositor, 1);
                }

                Atom wmState = XInternAtom(display, "_NET_WM_STATE", true);
                Atom wmStateFullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", true);

                XEvent event = {};
                uint64 mask = SubstructureNotifyMask | SubstructureRedirectMask;

                event.type = ClientMessage;
                event.xclient.window = window;
                event.xclient.format = 32;
                event.xclient.message_type = wmState;
                event.xclient.data.l[0] = 1;
                event.xclient.data.l[1] = wmStateFullscreen;
                event.xclient.data.l[2] = 0;
                event.xclient.data.l[3] = 1;
                XSendEvent(display, rootWindow, false, mask, &event);
                data.resizable = false;
            }
            else
            {
                //FIXME: Linux: Disable Fullscreen
                SetSize(data.width, data.height);
                SetStyle(WindowStyle::Titlebar | WindowStyle::MaximizeButton | WindowStyle::MinimizeButton);
                data.resizable = true;
            }
            data.fullscreen = fullscreen;
        }
    }
    void WindowImpl::SetIcon(strview path, int32 width, int32 height) noexcept
    {
        //TODO: Linux: Set Icon
    }
    void WindowImpl::SetTitle(std::string_view title) const noexcept
    {
        std::string _title(title.begin(), title.end());
        Atom wmName = XInternAtom(display, "_NET_WM_NAME", false);
        Atom u8string = XInternAtom(display, "UTF8_STRING", false);
        Atom wmIconName = XInternAtom(display, "_NET_WM_ICON_NAME", false);

        //Xutf8SetWMProperties(display, window, title.data(), title.data(), NULL, 0, NULL, NULL, NULL);
        
        XChangeProperty(display,  window, wmName, u8string, 8, PropModeReplace, (unsigned char*) title.data(), title.size());
        XChangeProperty(display,  window, wmIconName, u8string, 8, PropModeReplace, (unsigned char*)title.data(), title.size());

        XFlush(display);

        data.title = title;
    }
    void WindowImpl::SetPosition(uint32 x, uint32 y) const
    {
        XMoveWindow(display, window, x, y);
        data.position = Vec2u(x, y);
    }
    void WindowImpl::SetResizable(bool resizable)
    {
        if (resizable != data.resizable)
        {
            if (resizable)
            {
                XSizeHints* sizeHints = XAllocSizeHints();
                sizeHints->flags = PMinSize | PMaxSize;
                sizeHints->min_width = sizeHints->max_width = data.width;
                sizeHints->min_height = sizeHints->max_height = data.height;

                XSetWMNormalHints(display, window, sizeHints);
                XFree(sizeHints);
            }
            else
            {
                //FIXME: Temporary Solution! Recreating Window on SetResizable(true)
                Recreate(data.width, data.height, data.bitsPerPixel, data.title, data.sharedContext);
            }
        }
        data.resizable = resizable;
    }
    void WindowImpl::SetSize(uint32 width, uint32 height)
    {
        if (!data.resizable)
        {
            XSizeHints* sizeHints = XAllocSizeHints();
            sizeHints->flags = PMinSize | PMaxSize;
            sizeHints->min_width = sizeHints->max_width = width;
            sizeHints->min_height = sizeHints->max_height = height;
            XSetWMNormalHints(display, window, sizeHints);
            XFree(sizeHints);
        }

        XResizeWindow(display, window, width, height);
    }
    void WindowImpl::SetStyle(WindowStyle style)
    {
        if (style == WindowStyle::None)
        {
            Atom window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
            long value = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
            XChangeProperty(display, window, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&value, 1);
            
            return;
        }
        
        Atom wmHints = XInternAtom(display, "_MOTIF_WM_HINTS", false);
        unsigned char xStyle    = 0;
        unsigned char xFunction = 0;

        if (style & WindowStyle::Titlebar)
        {
            xStyle      |= borderStyle | titleStyle | minimizeStyle | menuStyle;
            xFunction   |= moveFunc | minimizeFunc;
        }

        const unsigned char hints[]
        {
            functionHint | decorationsHint,
            xFunction,
            xStyle,
            0,
            0
        };

        XChangeProperty(display, window, wmHints, wmHints, 32, PropModeReplace, hints, 5);
    }
    void WindowImpl::SetVisible(bool visible) const noexcept
    {
        visible ? XMapWindow(display, window) : XUnmapWindow(display, window);
    }

    void WindowImpl::ActivateContext() const
    {
        data.graphicsContext->Activate();
    }

    void WindowImpl::SetWMUserTime(::Time time)
    {
        if (time && time != lastUserActivityTime)
        {
            Atom userTime = XInternAtom(display, "_NET_WM_USER_TIME", true);
            if (userTime)
            {
                const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&time);
                XChangeProperty(display, window, userTime, XA_CARDINAL, 32, PropModeReplace, ptr, 1);
            }
            lastUserActivityTime = time;
        }
    }
    void WindowImpl::Recreate(int32 width, int32 height, int8 bitsPerPixel, strview title, IGraphicsContext* sharedContext)
    {
        Destroy();
        Create(width, height, bitsPerPixel, title, sharedContext);
    }
    void WindowImpl::Create(int32 width, int32 height, int8 bitsPerPixel, strview title, IGraphicsContext* sharedContext)
    {
        Visual* visual = DefaultVisual(display, DefaultScreen(display));
        int depth = DefaultDepth(display, DefaultScreen(display));
        XSetWindowAttributes attributes;
        int32 x = data.position.x;
        int32 y = data.position.y;
        unsigned long blackPixel = BlackPixel(display, DefaultScreen(display));

        long eventMask = ButtonMotionMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask;
        eventMask |= FocusChangeMask | KeyPressMask | KeyReleaseMask | LeaveWindowMask | PointerMotionMask;
        eventMask |= PropertyChangeMask | StructureNotifyMask | VisibilityChangeMask;

        attributes.colormap = XCreateColormap(display, rootWindow, visual, AllocNone);
        attributes.event_mask = eventMask;
        attributes.override_redirect = false;
        
        window = XCreateWindow(display, rootWindow, x, y, width, height, 0, depth, InputOutput, visual, CWEventMask | CWOverrideRedirect | CWColormap, &attributes);
        SetTitle(title);
        windowsCount++;
        (*GetWindowsMap())[window] = this;

        CreateGraphicsContext(bitsPerPixel, sharedContext);
        PostInitialize();
        
        SetVisible(true);
    }
    void WindowImpl::CreateGraphicsContext(uint8 bitsPerPixel, IGraphicsContext* sharedContext)
    {
        Graphics::IRendererAPI::Initialize();
        switch (Graphics::IRendererAPI::GetGraphicsAPI())
        {
            case Graphics::GraphicsAPI::OpenGL46:
            {
                data.graphicsContext = new Graphics::GL46Context(reinterpret_cast<void*>(&window), sharedContext);
                break;
            }
            case Graphics::GraphicsAPI::None:

                default:
                    VT_CORE_ASSERT_MSG(false, "Graphics API Not Supported!");
                    break;
        }
    }
    void WindowImpl::PostInitialize()
    {
        Atom wmProtocols = XInternAtom(display, "WM_PROTOCOLS", false);
        if (!wmProtocols) VTCoreLogError("Failed to acquire WM_PROTOCOLS Atom!");
        std::vector<Atom> atoms;

        Atom wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", false);
        if (wmDeleteWindow) atoms.push_back(wmDeleteWindow);
        else VTCoreLogError("Failed to acquire WM_DELETE_WINDOW Atom!");

        Atom wmPing = XInternAtom(display, "_NET_WM_PING", true);
        Atom wmPid = XInternAtom(display, "_NET_WM_PID", true);
        if (wmPing && wmPid)
        {
            const long pid = getpid();

            const unsigned char* pidPtr = reinterpret_cast<const unsigned char*>(&pid);
            XChangeProperty(display, window, wmPid, XA_CARDINAL, 32, PropModeReplace, pidPtr, 1);

            atoms.push_back(wmPing);
        }

        if (!atoms.empty())
        {
            const unsigned char* atomsPtr = reinterpret_cast<const unsigned char*>(atoms.data());
            XChangeProperty(display, window, wmProtocols, XA_ATOM, 32, PropModeReplace, atomsPtr, atoms.size());
        }
        else VTCoreLogError("Failure!");

        inputContext = XCreateIC(inputMethod, XNClientWindow, window, XNFocusWindow, window, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, nullptr);
        VT_CORE_ASSERT(inputContext != nullptr);

        Atom WMHintsAtom = XInternAtom(display, "_MOTIF_WM_HINTS", false);

        XWMHints* hints = XAllocWMHints();
        hints->flags         = StateHint;
        hints->initial_state = NormalState;
        XSetWMHints(display, window, hints);
        XFree(hints);
    }
    void WindowImpl::Destroy()
    {
        delete data.graphicsContext;
        GetWindowsMap()->erase(window);
        XDestroyIC(inputContext);
        XDestroyWindow(display, window);
        windowsCount--;
    }

    void WindowImpl::Initialize()
    {
        display = XOpenDisplay(nullptr);
        VT_CORE_ASSERT(display != nullptr);

        ewmhSupported = IsEwmhSupported(display);
        VT_CORE_ASSERT(ewmhSupported != false);

        inputMethod = XOpenIM(display, nullptr, nullptr, nullptr);
    }
    void WindowImpl::Shutdown()
    {
        XCloseIM(inputMethod);
        XCloseDisplay(display);
    }

    void WindowImpl::HandleEvent(XEvent event)
    {
        switch(event.type)
        {
            case ButtonPress:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xbutton.window];
                if (!window) return;
                switch (event.xbutton.button)
                {
                    case VT_XButtonLeft:
                        WindowEvents::mouseButtonPressedEvent(window, MouseCode::Left, false);
                        window->buttons[static_cast<uint32>(MouseCode::Left)] = true;
                        break;
                    case VT_XButtonMiddle:
                        WindowEvents::mouseButtonPressedEvent(window, MouseCode::Middle, false);
                        window->buttons[static_cast<uint32>(MouseCode::Middle)] = true;
                        break;
                    case VT_XButtonRight:
                        WindowEvents::mouseButtonPressedEvent(window, MouseCode::Right, false);
                        window->buttons[static_cast<uint32>(MouseCode::Right)] = true;
                        break;
                    case VT_XScrollXUp:
                        WindowEvents::mouseScrolledEvent(window, {1, 0});
                        break;
                    case VT_XScrollXDown:
                        WindowEvents::mouseScrolledEvent(window, {-1, 0});
                        break;
                    case VT_XScrollYUp:
                        WindowEvents::mouseScrolledEvent(window, {0, 1});
                        break;
                    case VT_XScrollYDown:
                        WindowEvents::mouseScrolledEvent(window, {0, -1});
                        break;
                    case VT_XButtonX1:
                        WindowEvents::mouseButtonPressedEvent(window, MouseCode::X1, false);
                        window->buttons[static_cast<uint32>(MouseCode::X1)] = true;
                        break;
                    case VT_XButtonX2:
                        WindowEvents::mouseButtonPressedEvent(window, MouseCode::X2, false);
                        window->buttons[static_cast<uint32>(MouseCode::X2)] = true;
                        break;
                }
                window->SetWMUserTime(event.xbutton.time);
                break;
            }
            case ButtonRelease:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xbutton.window];
                if (!window) return;
                MouseCode button = MouseCode::Unknown;
                switch (event.xbutton.button)
                {
                    case VT_XButtonLeft:
                        button = MouseCode::Left;
                        break;
                    case VT_XButtonMiddle:
                        button = MouseCode::Middle;
                        break;
                    case VT_XButtonRight:
                        button = MouseCode::Right;
                        break;
                    case VT_XButtonX1:
                        button = MouseCode::X1;
                        break;
                    case VT_XButtonX2:
                        button = MouseCode::X2;
                        break;
                }

                WindowEvents::mouseButtonReleasedEvent(window, button);
                window->buttons[static_cast<uint32>(button)] = false;
                break;
            }
            case ClientMessage:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xclient.window];
                if (!window) return;
                static Atom wmProtocols = XInternAtom(display, "WM_PROTOCOLS", false);

                if (event.xclient.message_type == wmProtocols)
                {
                    static Atom wmDeleteWindow  = XInternAtom(display, "WM_DELETE_WINDOW", false);
                    static Atom wmPing          = XInternAtom(display, "_NET_WM_PING", true);

                    // Window Closed Event
                    if (event.xclient.format == 32 && event.xclient.data.l[0] == static_cast<long>(wmDeleteWindow))
                    {
                        WindowEvents::windowClosedEvent(window);
                        window->data.isOpen = false;
                    }
                    else if (wmPing && event.xclient.format == 32 && event.xclient.data.l[0] == static_cast<long>(wmPing))
                    {
                        // Send _NET_WM_PING event back to root window to indicate that window is still responding
                        event.xclient.window = DefaultRootWindow(display);
                        XSendEvent(display, DefaultRootWindow(display), false, SubstructureNotifyMask | SubstructureRedirectMask, &event);
                    }

                }
                break;
            }
            case ConfigureNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xconfigure.window];
                if (!window) return;
                // Window Resized Event
                if (event.xconfigure.width != window->data.width || event.xconfigure.height != window->data.height)
                {
                    WindowEvents::windowResizedEvent(window, Math::Vec2u(event.xconfigure.width, event.xconfigure.height));
                    window->data.width = event.xconfigure.width;
                    window->data.height = event.xconfigure.height;
                }
                break;
            }
            case EnterNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xcrossing.window];
                if (!window) return;
                window->data.mouseHovered = true;
            }
            case FocusIn:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xfocus.window];
                if (!window) return;
                if (window->inputContext) XSetICFocus(window->inputContext);
                WindowEvents::focusChangedEvent(window, true);
                break;
            }
            case FocusOut:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xfocus.window];
                if (!window) return;
                if (window->inputContext) XUnsetICFocus(window->inputContext);
                WindowEvents::focusChangedEvent(window, false);
                break;
            }
            case KeyPress:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xkey.window];
                if (!window) return;
                KeyCode key = KeyCode::Unknown;
                for (int i = 0; i < 4; i++)
                {
                    key = TranslateKeyCode(XLookupKeysym(&event.xkey, i));
                    if (key != KeyCode::Unknown) break;
                }

                WindowEvents::keyPressedEvent(window, key, 0);
                window->keys[static_cast<uint32>(key)] = true;

                if (!XFilterEvent(&event, XNone))
                {
                    if (window->inputContext)
                    {
                        Status status;
                        uint8  keyBuffer[16];

                        int32 length = Xutf8LookupString
                        (
                            window->inputContext,
                            &event.xkey,
                            reinterpret_cast<char*>(keyBuffer),
                            sizeof(keyBuffer),
                            nullptr,
                            &status
                        );

                        if (length > 0)
                        {
                            WindowEvents::keyTypedEvent(window, keyBuffer[0]);
                        }
                    }
                }
                window->SetWMUserTime(event.xkey.time);
                break;
            }
            case KeyRelease:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xkey.window];
                if (!window) return;
                KeyCode key = KeyCode::Unknown;

                for (int i = 0; i < 4; i++)
                {
                    key = TranslateKeyCode(XLookupKeysym(&event.xkey, i));
                    if (key != KeyCode::Unknown) break;
                }

                WindowEvents::keyReleasedEvent(window, key);
                window->keys[static_cast<uint32>(key)] = false;
                break;
            }
            case LeaveNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xcrossing.window];
                if (!window) return;

                window->data.mouseHovered = false;
            }
            case MotionNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xmotion.window];
                if (!window) return;
                WindowEvents::mouseMovedEvent(window, {(float)event.xmotion.x, (float)event.xmotion.y});
                window->data.mousePosition.x = event.xmotion.x;
                window->data.mousePosition.y = event.xmotion.y;
                break;
            }
            case PropertyNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xproperty.window];
                if (!window) return;
                if (window->lastUserActivityTime) window->lastUserActivityTime = event.xproperty.time;
                break;
            }
            case UnmapNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xunmap.window];
                if (!window) return;
                window->data.visible = false;
                break;
            }
            case VisibilityNotify:
            {
                WindowImpl* window = (*GetWindowsMap())[event.xmap.window];
                if (!window) return;
                window->data.visible = true;
                break;
            }
            

            default:
                break;
        }
    }
}
#endif
