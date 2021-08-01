//
// Created by Vitriol1744 on 22.06.2021.
//

#include "vtpch.hpp"
#include "Vortex/Core/PlatformInit.hpp"

#ifdef VT_PLATFORM_WINDOWS
//#define _UNICODE
#include "Win32Window.hpp"

#include "Vortex/Input/KeyCode.hpp"
#include "Vortex/Input/MouseCode.hpp"
#include "Vortex/Graphics/API/OpenGL46/GL46Context.hpp"
#include "Vortex/Graphics/API/IRendererAPI.hpp"

using namespace Vortex::Math;
using namespace Vortex::Input;
using namespace Vortex::Graphics;

#define VTCreateWindow(width, height, title, style, x, y) \
    CreateWindowExW(0, windowClassName, title.data(), style, x, y, width, height, nullptr, nullptr, hInstance, nullptr)

namespace Vortex
{
    namespace
    {
        const wchar_t* windowClassName = L"Window Class";
        HINSTANCE hInstance = nullptr;

        KeyCode     VTKeyCode(uint32 keycode, bool extended = false)
        {
            switch (keycode)
            {
                case '0':           return KeyCode::Num0;
                case '1':           return KeyCode::Num1;
                case '2':           return KeyCode::Num2;
                case '3':           return KeyCode::Num3;
                case '4':           return KeyCode::Num4;
                case '5':           return KeyCode::Num5;
                case '6':           return KeyCode::Num6;
                case '7':           return KeyCode::Num7;
                case '8':           return KeyCode::Num8;
                case '9':           return KeyCode::Num9;
                case 'A':           return KeyCode::A;
                case 'B':           return KeyCode::B;
                case 'C':           return KeyCode::C;
                case 'D':           return KeyCode::D;
                case 'E':           return KeyCode::E;
                case 'F':           return KeyCode::F;
                case 'G':           return KeyCode::G;
                case 'H':           return KeyCode::H;
                case 'I':           return KeyCode::I;
                case 'J':           return KeyCode::J;
                case 'K':           return KeyCode::K;
                case 'L':           return KeyCode::L;
                case 'M':           return KeyCode::M;
                case 'N':           return KeyCode::N;
                case 'O':           return KeyCode::O;
                case 'P':           return KeyCode::P;
                case 'Q':           return KeyCode::Q;
                case 'R':           return KeyCode::R;
                case 'S':           return KeyCode::S;
                case 'T':           return KeyCode::T;
                case 'U':           return KeyCode::U;
                case 'V':           return KeyCode::V;
                case 'W':           return KeyCode::W;
                case 'X':           return KeyCode::X;
                case 'Y':           return KeyCode::Y;
                case 'Z':           return KeyCode::Z;
                case VK_OEM_3:      return KeyCode::Tilde;
                case VK_F1:         return KeyCode::F1;
                case VK_F2:         return KeyCode::F2;
                case VK_F3:         return KeyCode::F3;
                case VK_F4:         return KeyCode::F4;
                case VK_F5:         return KeyCode::F5;
                case VK_F6:         return KeyCode::F6;
                case VK_F7:         return KeyCode::F7;
                case VK_F8:         return KeyCode::F8;
                case VK_F9:         return KeyCode::F9;
                case VK_F10:        return KeyCode::F10;
                case VK_F11:        return KeyCode::F11;
                case VK_F12:        return KeyCode::F12;
                case VK_F13:        return KeyCode::F13;
                case VK_F14:        return KeyCode::F14;
                case VK_F15:        return KeyCode::F15;
                case VK_ESCAPE:     return KeyCode::Escape;
                case VK_BACK:       return KeyCode::BackSpace;
                case VK_TAB:        return KeyCode::Tab;
                case VK_CAPITAL:    return KeyCode::CapsLock;
                case VK_RETURN:     return extended ? KeyCode::Enter : KeyCode::Return;
                case VK_LSHIFT:     return KeyCode::LShift;
                case VK_RSHIFT:     return KeyCode::RShift;
                case VK_LCONTROL:   return KeyCode::LCtrl;
                case VK_RCONTROL:   return KeyCode::RCtrl;
                case VK_MENU:       return extended ? KeyCode::RAlt : KeyCode::LAlt;
                case VK_LWIN:       return KeyCode::LSystem;
                case VK_RWIN:       return KeyCode::RSystem;
                case VK_SPACE:      return KeyCode::Space;
                case VK_OEM_MINUS:  return KeyCode::Hyphen;
                case VK_OEM_PLUS:   return KeyCode::Equal;
                case VK_DECIMAL:    return KeyCode::Decimal;
                case VK_OEM_4:      return KeyCode::LBracket;
                case VK_OEM_6:      return KeyCode::RBracket;
                case VK_OEM_1:      return KeyCode::Semicolon;
                case VK_OEM_7:      return KeyCode::Apostrophe;
                case VK_OEM_COMMA:  return KeyCode::Comma;
                case VK_OEM_PERIOD: return KeyCode::Period;
                case VK_OEM_2:      return KeyCode::Slash;
                case VK_OEM_5:      return KeyCode::BackSlash;
                case VK_UP:         return KeyCode::Up;
                case VK_DOWN:       return KeyCode::Down;
                case VK_LEFT:       return KeyCode::Left;
                case VK_RIGHT:      return KeyCode::Right;
                case VK_NUMPAD0:    return KeyCode::Numpad0;
                case VK_NUMPAD1:    return KeyCode::Numpad1;
                case VK_NUMPAD2:    return KeyCode::Numpad2;
                case VK_NUMPAD3:    return KeyCode::Numpad3;
                case VK_NUMPAD4:    return KeyCode::Numpad4;
                case VK_NUMPAD5:    return KeyCode::Numpad5;
                case VK_NUMPAD6:    return KeyCode::Numpad6;
                case VK_NUMPAD7:    return KeyCode::Numpad7;
                case VK_NUMPAD8:    return KeyCode::Numpad8;
                case VK_NUMPAD9:    return KeyCode::Numpad9;
                case VK_SEPARATOR:  return KeyCode::Separator;
                case VK_ADD:        return KeyCode::Add;
                case VK_SUBTRACT:   return KeyCode::Subtract;
                case VK_MULTIPLY:   return KeyCode::Multiply;
                case VK_DIVIDE:     return KeyCode::Divide;
                case VK_INSERT:     return KeyCode::Insert;
                case VK_DELETE:     return KeyCode::Delete;
                case VK_PRIOR:      return KeyCode::PageUp;
                case VK_NEXT:       return KeyCode::PageDown;
                case VK_HOME:       return KeyCode::Home;
                case VK_END:        return KeyCode::End;
            }

            return KeyCode::Unknown;
        }
        MouseCode   VTMouseCode(uint32 mousecode)
        {
            switch (mousecode)
            {
                case MK_LBUTTON:    return MouseCode::Left;
                case MK_MBUTTON:    return MouseCode::Middle;
                case MK_RBUTTON:    return MouseCode::Right;
                case MK_XBUTTON1:   return MouseCode::X1;
                case MK_XBUTTON2:   return MouseCode::X2;
            }

            return MouseCode::Unknown;
        }
    }

    unsigned int WindowImpl::windowsCount = 0;

    WindowImpl::WindowImpl(uint32 width, uint32 height, uint32 bitsPerPixel, std::wstring_view title, Ref<IWindow> share)
    { 
        if (!windowsCount) Initialize();

        uint32 style = WS_OVERLAPPEDWINDOW;
        RECT wrect;
        wrect.left      = 0;
        wrect.right     = width;
        wrect.top       = 0;
        wrect.bottom    = height;
        AdjustWindowRectEx(&wrect, style, false, 0);

        HDC dc = GetDC(NULL);

        data.width  = wrect.right - wrect.left;
        data.height = wrect.bottom - wrect.top;
        data.title = title;
        data.position = Vec2u(wrect.left, wrect.top);
        int x = (GetDeviceCaps(dc, HORZRES) - static_cast<int>(width)) / 2;
        int y = (GetDeviceCaps(dc, VERTRES) - static_cast<int>(height)) / 2;
        
        hWnd = VTCreateWindow(data.width, height, title, style, x, y);
        (*GetWindowsMap())[hWnd] = this;
        memset(keys, 0, sizeof(bool) * static_cast<uint32>(KeyCode::KeysCount));

        VT_CORE_LOG_TRACE("Window Created! width: {}, height: {}", data.width, data.height);

        IRendererAPI::Initialize();
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
            {
                //TODO: Set Bits Per Pixel
                data.graphicsContext = new GL46Context(reinterpret_cast<void*>(hWnd), share ? share->GetGraphicsContext() : nullptr);
                break;
            }
            case GraphicsAPI::None:

            default:
                VT_CORE_LOG_FATAL("Graphics API Not Supported!");
                break;
        }

        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }
    WindowImpl::~WindowImpl()
    {
        GetWindowsMap()->erase(hWnd);
        DestroyWindow(hWnd);
        windowsCount--;
        if (!windowsCount) Shutdown();
    }

    void WindowImpl::Update()
    {
        MSG msg;
        while (PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    void WindowImpl::Present()
    {
        data.graphicsContext->Present();
    }

    void WindowImpl::ShowCursor() const noexcept
    {
        ::ShowCursor(true);
    }
    void WindowImpl::HideCursor() const noexcept
    {
        ::ShowCursor(false);
    }
    void WindowImpl::RequestFocus() const
    {
        SetForegroundWindow(hWnd);
    }
    void WindowImpl::SetFullscreen(bool fullscreen)
    {
        if (fullscreen != data.fullscreen)
        {
            int x, y, width, height;
            if (fullscreen)
            {
                SetWindowLongW(hWnd, GWL_STYLE, 0);

                HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFOEXW monitorInfo = {};
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                GetMonitorInfoW(hMonitor, &monitorInfo);

                x       = monitorInfo.rcMonitor.left;
                y       = monitorInfo.rcMonitor.top;
                width   = monitorInfo.rcMonitor.right - x;
                height  = monitorInfo.rcMonitor.bottom - y;

                //DEVMODEW devMode;
                //devMode.dmSize = sizeof(devMode);
                //devMode.dmPelsWidth = width;
                //devMode.dmPelsHeight = height;
                //devMode.dmBitsPerPel = data.bitsPerPixel;
                //devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
                //

                //if (ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                //{
                //    VT_CORE_LOG_ERROR("Failed to change display mode for fullscreen");
                //    return;
                //}

                SetWindowPos(hWnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED | SWP_NOACTIVATE);
                SetSize(width, height);
                ShowWindow(hWnd, SW_MAXIMIZE);
            }
            else
            {
                x       = data.position.x;
                y       = data.position.y;
                width   = data.width;
                height  = data.height;
                SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowPos(hWnd, HWND_NOTOPMOST, x, y, width, height, SWP_FRAMECHANGED | SWP_NOACTIVATE);
                ShowWindow(hWnd, SW_NORMAL);
            }
            data.fullscreen = fullscreen;
        }
    }
    void WindowImpl::SetIcon(std::string_view path, int32 width, int32 height) const
    {
        const std::wstring& name = std::wstring(path.begin(), path.end());
        auto icon = (LONG_PTR)LoadImageW(nullptr, name.c_str(), IMAGE_ICON, width, height, LR_LOADFROMFILE);
        //TODO: Refactor This
        //SetClassLongPtrW(hWnd, GCLP_HICON, icon);
        //SetClassLongPtrW(hWnd, GCLP_HICONSM, icon);
        SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
        SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    }
    void WindowImpl::SetTitle(std::string_view title) const noexcept
    {
        SetWindowTextA(hWnd, title.data());
        data.title = std::wstring(title.begin(), title.end());
    }
    void WindowImpl::SetTitle(std::wstring_view title) const noexcept
    {
        SetWindowTextW(hWnd, title.data());
        data.title = title;
    }
    void WindowImpl::SetPosition(uint32 x, uint32 y) const
    {
        SetWindowPos(hWnd, HWND_TOP, x, y, data.width, data.height, SWP_NOSIZE);
        data.position = { x, y };
    }
    void WindowImpl::SetResizable(bool resizable)
    {
        data.resizable = resizable;
        uint32 style = WS_OVERLAPPEDWINDOW;
        if (!resizable) style ^= WS_THICKFRAME;
        SetWindowLongPtrW(hWnd, GWL_STYLE, style);
        ShowWindow(hWnd, data.visible ? SW_SHOW : SW_HIDE);
    }
    void WindowImpl::SetSize(uint32 width, uint32 height)
    {
        RECT wrect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
        AdjustWindowRect(&wrect, GetWindowLong(hWnd, GWL_STYLE), false);

        width = wrect.right - wrect.left;
        height = wrect.bottom - wrect.top;

        SetWindowPos(hWnd, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
    }
    void WindowImpl::SetVisible(bool visible) const
    {
        ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
        data.visible = visible;
    }

    void WindowImpl::ActivateContext() const
    {
        data.graphicsContext->Activate();
    }

    void WindowImpl::Initialize()
    {
        hInstance = GetModuleHandle(nullptr);

        WNDCLASSEXW wcex{};

        //TODO: Default Icon?
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = HandleGlobalEvents;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
        wcex.hIcon = nullptr;
        wcex.hbrBackground = nullptr;
        wcex.hInstance = hInstance;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = windowClassName;

        RegisterClassExW(&wcex);
    }
    void WindowImpl::Shutdown()
    {
        if (!windowsCount) return;
        UnregisterClassW(windowClassName, hInstance);
    }

    LRESULT WINAPI WindowImpl::HandleGlobalEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        WindowImpl* window = (*GetWindowsMap())[hWnd];
        return window ? window->HandleEvents(hWnd, msg, wParam, lParam) : DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    LRESULT WINAPI WindowImpl::HandleEvents(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        int mask = 1 << 24;

        switch (msg)
        {
            case WM_CHAR:
            {
                uint32 c = static_cast<uint32>(wParam);
                keyTypedEvent(c);
                break;
            }
            case WM_CLOSE:
            case WM_DESTROY:
                data.isOpen = false;
                PostQuitMessage(0);
                break;
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                KeyCode key = VTKeyCode(static_cast<uint32>(wParam), lParam & (1 << 24));
                if (((uint32)lParam & (1 << 31)) == 0)
                {
                    keyPressedEvent(key, lParam & 0xFF00);
                    keys[static_cast<uint32>(key)] = true;
                }
                else
                {
                    keyReleasedEvent(key);
                    keys[static_cast<uint32>(key)] = false;
                }
                break;
            }
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_XBUTTONUP:
                mouseButtonReleasedEvent(VTMouseCode(wParam));
                break;
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_XBUTTONDOWN:
                mouseButtonPressedEvent(VTMouseCode(wParam), false);
                break;
            case WM_LBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_XBUTTONDBLCLK:
                mouseButtonPressedEvent(VTMouseCode(wParam), true);
                break;
            case WM_MOUSEWHEEL:
            {
                int16 delta = HIWORD(wParam);
                mouseScrolledEvent({ delta / 120.0f, 0.0f });
                break;
            }
            case WM_MOUSEHWHEEL:
            {
                int16 delta = HIWORD(wParam);
                mouseScrolledEvent({ 0.0f, delta / 120.0f });
                break;
            }
            case WM_MOUSEMOVE:
                POINT point = (POINT)lParam;
                ScreenToClient(hWnd, &point);
                data.position = { point.x, point.y };
                mouseMovedEvent({ point.x, point.y });
                break;
            case WM_SIZE:
                data.width = LOWORD(lParam);
                data.height = HIWORD(lParam);
                windowResizedEvent(static_cast<WindowResizedEventType>(wParam), { LOWORD(lParam), HIWORD(lParam) });
                //TODO: Change Viewport?
                break;
            case WM_SETFOCUS:
                focusChangedEvent(true);
                break;
            case WM_KILLFOCUS:
                focusChangedEvent(false);
                break;

            default:
                break;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}
#endif