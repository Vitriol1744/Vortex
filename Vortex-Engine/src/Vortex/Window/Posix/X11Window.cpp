//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"

#include "Graphics/API/OpenGL46/GL46Context.hpp"
#include "Graphics/API/IRendererAPI.hpp"

#include <cstdio>

#ifdef VT_PLATFORM_LINUX
#include "X11Window.hpp"
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>

namespace Vortex
{
    using namespace Input;
    using namespace Graphics;
    using namespace Math;
    
    namespace
    {
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
                case XK_Caps_Lock:      return KeyCode::Capslock;
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
                case XK_backslash:      return KeyCode::Backslash;
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

                default:
                    return KeyCode::Unknown;
            }
        }
        #pragma clang diagnostic pop
    }

    uint32      WindowImpl::windowsCount    = 0;
    Display*    WindowImpl::display         = nullptr;
    XIM         WindowImpl::inputMethod     = nullptr;
    Cursor      WindowImpl::blankCursor     = 0;

    WindowImpl::WindowImpl(int32 width, int32 height, std::wstring_view title, Ref<IWindow> share)
    {
        data.position.x = 0;
        data.position.y = 0;
        if (windowsCount == 0) Initialize();

        window = XCreateSimpleWindow
        (
            display, DefaultRootWindow(display),
            data.position.x, data.position.y, 800, 600, 0, BlackPixel(display, DefaultScreen(display)),
            BlackPixel(display, DefaultScreen(display))
        );
        windowsCount++;
        (*GetWindowsMap())[window] = this;

        Graphics::IRendererAPI::Initialize();
        switch (Graphics::IRendererAPI::GetGraphicsAPI())
        {
            case Graphics::GraphicsAPI::OpenGL46:
            {
                data.graphicsContext = new Graphics::GL46Context(reinterpret_cast<void*>(&window), share ? share->GetGraphicsContext() : nullptr);
                break;
            }
            case Graphics::GraphicsAPI::None:

            default:
                VT_CORE_ASSERT_MSG(false, "Graphics API Not Supported!");
                break;
        }

        inputContext = XCreateIC(inputMethod, XNClientWindow, window, XNFocusWindow, window, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);
        VT_CORE_ASSERT(inputContext != nullptr);
        XSelectInput
        (
            display, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
            StructureNotifyMask
        );
        XMapWindow(display, window);

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
        XDestroyIC(inputContext);
        XDestroyWindow(display, window);
        windowsCount--;
        if (windowsCount == 0) Shutdown();
    }

    void WindowImpl::Update()
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
        XDefineCursor(display, window, 0L);
    }
    void WindowImpl::HideCursor() const noexcept
    {
        XDefineCursor(display, window, blankCursor);
    }
    void WindowImpl::SetFullscreen(bool fullscreen) const noexcept
    {
        if (fullscreen)
        {
            //TODO: Handle fullscreen
        }
        else
        {
        
        }
    }
    void WindowImpl::SetIcon(std::string_view path) const noexcept
    {
        //TODO: Set Icon
    }
    void WindowImpl::SetTitle(std::string_view title) const noexcept
    {
        XStoreName(display, window, title.data());
    }
    void WindowImpl::SetTitle(std::wstring_view title) const noexcept
    {
        //TODO: wide strings support
        SetTitle(std::string(title.begin(), title.end()));
    }
    void WindowImpl::SetPosition(uint32 x, uint32 y) const
    {
        XMoveWindow(display, window, x, y);
        //TODO:data.position = Vec2u(x, y);
    }
    void WindowImpl::SetVisible(bool visible) const noexcept
    {
        //TODO: Implement This:
    }

    void WindowImpl::ActivateContext() const
    {
        data.graphicsContext->Activate();
    }

    void WindowImpl::Initialize()
    {
        display = XOpenDisplay(nullptr);
        VT_CORE_ASSERT(display != nullptr);

        inputMethod = XOpenIM(display, nullptr, nullptr, nullptr);
    }
    void WindowImpl::Shutdown()
    {
        XCloseDisplay(display);
    }

    //TODO: Add More Events
    void WindowImpl::HandleEvent(XEvent event)
    {
        switch(event.type)
        {
            case KeyPress:
            {
                KeyCode key = KeyCode::Unknown;
                for (int i = 0; i < 4; i++)
                {
                    key = TranslateKeyCode(XLookupKeysym(&event.xkey, i));
                    if (key != KeyCode::Unknown) break;
                }

                (*GetWindowsMap())[event.xkey.window]->keyPressedEvent({key, 0, {0, 0}});
                (*GetWindowsMap())[event.xkey.window]->keys[static_cast<uint32>(key)] = true;

                if (!XFilterEvent(&event, 0L))
                {
                    if ((*GetWindowsMap())[event.xkey.window]->inputContext)
                    {
                        Status status;
                        uint8  keyBuffer[16];

                        int32 length = Xutf8LookupString
                        (
                            (*GetWindowsMap())[event.xkey.window]->inputContext,
                            &event.xkey,
                            reinterpret_cast<char*>(keyBuffer),
                            sizeof(keyBuffer),
                            nullptr,
                            &status
                        );

                        if (length > 0)
                        {
                            (*GetWindowsMap())[event.xkey.window]->keyTypedEvent(keyBuffer[0]);
                        }
                    }
                }
                break;
            }
            case KeyRelease:
            {
                KeyCode key = KeyCode::Unknown;

                for (int i = 0; i < 4; i++)
                {
                    key = TranslateKeyCode(XLookupKeysym(&event.xkey, i));
                    if (key != KeyCode::Unknown) break;
                }

                (*GetWindowsMap())[event.xkey.window]->keyReleasedEvent(key, {0, 0});
                (*GetWindowsMap())[event.xkey.window]->keys[static_cast<uint32>(key)] = false;
                break;
            }
            case ButtonPress:
            {
                switch (event.xbutton.button)
                {
                    case Button1:
                        (*GetWindowsMap())[event.xbutton.window]->mouseButtonPressedEvent(MouseCode::Left, {(float)event.xbutton.x, (float)event.xbutton.y});
                        (*GetWindowsMap())[event.xkey.window]->buttons[static_cast<uint32>(MouseCode::Left)] = true;
                        break;
                    case Button2:
                        (*GetWindowsMap())[event.xbutton.window]->mouseButtonPressedEvent(MouseCode::Middle, {(float)event.xbutton.x, (float)event.xbutton.y});
                        (*GetWindowsMap())[event.xkey.window]->buttons[static_cast<uint32>(MouseCode::Middle)] = true;
                        break;
                    case Button3:
                        (*GetWindowsMap())[event.xbutton.window]->mouseButtonPressedEvent(MouseCode::Right, {(float)event.xbutton.x, (float)event.xbutton.y});
                        (*GetWindowsMap())[event.xkey.window]->buttons[static_cast<uint32>(MouseCode::Right)] = true;
                        break;
                    case Button4:
                        (*GetWindowsMap())[event.xbutton.window]->mouseScrolledEvent({1, 0}, {(float)event.xbutton.x, (float)event.xbutton.y});
                        break;
                    case Button5:
                        (*GetWindowsMap())[event.xbutton.window]->mouseScrolledEvent({-1, 0}, {(float)event.xbutton.x, (float)event.xbutton.y});
                        break;
                    case 6:
                        (*GetWindowsMap())[event.xbutton.window]->mouseScrolledEvent({0, 1}, {(float)event.xbutton.x, (float)event.xbutton.y});
                        break;
                    case 7:
                        (*GetWindowsMap())[event.xbutton.window]->mouseScrolledEvent({0, -1}, {(float)event.xbutton.x, (float)event.xbutton.y});
                        break;
                    case 8:
                        (*GetWindowsMap())[event.xbutton.window]->mouseButtonPressedEvent(MouseCode::X1, {(float)event.xbutton.x, (float)event.xbutton.y});
                        (*GetWindowsMap())[event.xkey.window]->buttons[static_cast<uint32>(MouseCode::X1)] = true;
                        break;
                    case 9:
                        (*GetWindowsMap())[event.xbutton.window]->mouseButtonPressedEvent(MouseCode::X2, {(float)event.xbutton.x, (float)event.xbutton.y});
                        (*GetWindowsMap())[event.xkey.window]->buttons[static_cast<uint32>(MouseCode::X2)] = true;
                        break;
                }
                break;
            }
            case ButtonRelease:
            {
                MouseCode button = MouseCode::Unknown;
                switch (event.xbutton.button)
                {
                    case Button1:
                        button = MouseCode::Left;
                        break;
                    case Button2:
                        button = MouseCode::Middle;
                        break;
                    case Button3:
                        button = MouseCode::Right;
                        break;
                    case 8:
                        button = MouseCode::X1;
                        break;
                    case 9:
                        button = MouseCode::X2;
                        break;
                }

                (*GetWindowsMap())[event.xbutton.window]->mouseButtonReleasedEvent(button, {(float)event.xbutton.x, (float)event.xbutton.y});
                (*GetWindowsMap())[event.xkey.window]->buttons[static_cast<uint32>(button)] = false;
                
                break;
            }
            case ClientMessage:
            {
                
                break;
            }
            case MotionNotify:
                (*GetWindowsMap())[event.xbutton.window]->mouseMovedEvent({(float)event.xmotion.x_root, (float)event.xmotion.y_root});
                break;

            default:
                break;
        }
    }
}
#endif
