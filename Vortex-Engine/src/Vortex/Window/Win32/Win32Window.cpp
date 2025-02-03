/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Window/Win32/Win32Window.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Utf.hpp"

#include <shellscalingapi.h>

namespace Vortex
{
    usize                                  Win32Window::s_WindowsCount = 0;
    inline static constexpr const wchar_t* s_WindowClassName
        = L"Vortex Window Class";

    static void glfwErrorCallback(i32 code, const char* description)
    {
        (void)code;
        (void)description;
        VtCoreError(
            "GLFW: An error has occurred, ErrorCode: {}, Description: {}", code,
            description);
    }

    using Input::KeyCode;
    static KeyCode ToVtKeyCode(i32 key)
    {
        KeyCode ret = KeyCode::eUnknown;
        switch (key)
        {
            case GLFW_KEY_0: ret = KeyCode::eNum0; break;
            case GLFW_KEY_1: ret = KeyCode::eNum1; break;
            case GLFW_KEY_2: ret = KeyCode::eNum2; break;
            case GLFW_KEY_3: ret = KeyCode::eNum3; break;
            case GLFW_KEY_4: ret = KeyCode::eNum4; break;
            case GLFW_KEY_5: ret = KeyCode::eNum5; break;
            case GLFW_KEY_6: ret = KeyCode::eNum6; break;
            case GLFW_KEY_7: ret = KeyCode::eNum7; break;
            case GLFW_KEY_8: ret = KeyCode::eNum8; break;
            case GLFW_KEY_9: ret = KeyCode::eNum9; break;
            case GLFW_KEY_A: ret = KeyCode::eA; break;
            case GLFW_KEY_B: ret = KeyCode::eB; break;
            case GLFW_KEY_C: ret = KeyCode::eC; break;
            case GLFW_KEY_D: ret = KeyCode::eD; break;
            case GLFW_KEY_E: ret = KeyCode::eE; break;
            case GLFW_KEY_F: ret = KeyCode::eF; break;
            case GLFW_KEY_G: ret = KeyCode::eG; break;
            case GLFW_KEY_H: ret = KeyCode::eH; break;
            case GLFW_KEY_I: ret = KeyCode::eI; break;
            case GLFW_KEY_J: ret = KeyCode::eJ; break;
            case GLFW_KEY_K: ret = KeyCode::eK; break;
            case GLFW_KEY_L: ret = KeyCode::eL; break;
            case GLFW_KEY_M: ret = KeyCode::eM; break;
            case GLFW_KEY_N: ret = KeyCode::eN; break;
            case GLFW_KEY_O: ret = KeyCode::eO; break;
            case GLFW_KEY_P: ret = KeyCode::eP; break;
            case GLFW_KEY_Q: ret = KeyCode::eQ; break;
            case GLFW_KEY_R: ret = KeyCode::eR; break;
            case GLFW_KEY_S: ret = KeyCode::eS; break;
            case GLFW_KEY_T: ret = KeyCode::eT; break;
            case GLFW_KEY_U: ret = KeyCode::eU; break;
            case GLFW_KEY_V: ret = KeyCode::eV; break;
            case GLFW_KEY_W: ret = KeyCode::eW; break;
            case GLFW_KEY_X: ret = KeyCode::eX; break;
            case GLFW_KEY_Y: ret = KeyCode::eY; break;
            case GLFW_KEY_Z: ret = KeyCode::eZ; break;
            case GLFW_KEY_GRAVE_ACCENT: ret = KeyCode::eTilde; break;
            case GLFW_KEY_F1: ret = KeyCode::eF1; break;
            case GLFW_KEY_F2: ret = KeyCode::eF2; break;
            case GLFW_KEY_F3: ret = KeyCode::eF3; break;
            case GLFW_KEY_F4: ret = KeyCode::eF4; break;
            case GLFW_KEY_F5: ret = KeyCode::eF5; break;
            case GLFW_KEY_F6: ret = KeyCode::eF6; break;
            case GLFW_KEY_F7: ret = KeyCode::eF7; break;
            case GLFW_KEY_F8: ret = KeyCode::eF8; break;
            case GLFW_KEY_F9: ret = KeyCode::eF9; break;
            case GLFW_KEY_F10: ret = KeyCode::eF10; break;
            case GLFW_KEY_F11: ret = KeyCode::eF11; break;
            case GLFW_KEY_F12: ret = KeyCode::eF12; break;
            case GLFW_KEY_F13: ret = KeyCode::eF13; break;
            case GLFW_KEY_F14: ret = KeyCode::eF14; break;
            case GLFW_KEY_F15: ret = KeyCode::eF15; break;
            case GLFW_KEY_ESCAPE: ret = KeyCode::eEscape; break;
            case GLFW_KEY_BACKSPACE: ret = KeyCode::eBackspace; break;
            case GLFW_KEY_TAB: ret = KeyCode::eTab; break;
            case GLFW_KEY_CAPS_LOCK: ret = KeyCode::eCapsLock; break;
            case GLFW_KEY_ENTER: ret = KeyCode::eReturn; break;
            case GLFW_KEY_KP_ENTER: ret = KeyCode::eEnter; break;
            case GLFW_KEY_LEFT_SHIFT: ret = KeyCode::eLShift; break;
            case GLFW_KEY_RIGHT_SHIFT: ret = KeyCode::eRShift; break;
            case GLFW_KEY_LEFT_CONTROL: ret = KeyCode::eLCtrl; break;
            case GLFW_KEY_RIGHT_CONTROL: ret = KeyCode::eRCtrl; break;
            case GLFW_KEY_LEFT_ALT: ret = KeyCode::eLAlt; break;
            case GLFW_KEY_RIGHT_ALT: ret = KeyCode::eRAlt; break;
            case GLFW_KEY_LEFT_SUPER: ret = KeyCode::eLSystem; break;
            case GLFW_KEY_RIGHT_SUPER: ret = KeyCode::eRSystem; break;
            case GLFW_KEY_SPACE: ret = KeyCode::eSpace; break;
            case GLFW_KEY_MINUS: ret = KeyCode::eHyphen; break;
            case GLFW_KEY_EQUAL: ret = KeyCode::eEqual; break;
            case GLFW_KEY_KP_DECIMAL: ret = KeyCode::eDecimal; break;
            case GLFW_KEY_LEFT_BRACKET: ret = KeyCode::eLBracket; break;
            case GLFW_KEY_RIGHT_BRACKET: ret = KeyCode::eRBracket; break;
            case GLFW_KEY_SEMICOLON: ret = KeyCode::eSemicolon; break;
            case GLFW_KEY_APOSTROPHE: ret = KeyCode::eApostrophe; break;
            case GLFW_KEY_COMMA: ret = KeyCode::eComma; break;
            case GLFW_KEY_PERIOD: ret = KeyCode::ePeriod; break;
            case GLFW_KEY_SLASH: ret = KeyCode::eSlash; break;
            case GLFW_KEY_BACKSLASH: ret = KeyCode::eBackslash; break;
            case GLFW_KEY_UP: ret = KeyCode::eUp; break;
            case GLFW_KEY_DOWN: ret = KeyCode::eDown; break;
            case GLFW_KEY_LEFT: ret = KeyCode::eLeft; break;
            case GLFW_KEY_RIGHT: ret = KeyCode::eRight; break;
            case GLFW_KEY_KP_0: ret = KeyCode::eNumpad0; break;
            case GLFW_KEY_KP_1: ret = KeyCode::eNumpad1; break;
            case GLFW_KEY_KP_2: ret = KeyCode::eNumpad2; break;
            case GLFW_KEY_KP_3: ret = KeyCode::eNumpad3; break;
            case GLFW_KEY_KP_4: ret = KeyCode::eNumpad4; break;
            case GLFW_KEY_KP_5: ret = KeyCode::eNumpad5; break;
            case GLFW_KEY_KP_6: ret = KeyCode::eNumpad6; break;
            case GLFW_KEY_KP_7: ret = KeyCode::eNumpad7; break;
            case GLFW_KEY_KP_8: ret = KeyCode::eNumpad8; break;
            case GLFW_KEY_KP_9: ret = KeyCode::eNumpad9; break;
            case GLFW_KEY_KP_ADD: ret = KeyCode::eAdd; break;
            case GLFW_KEY_KP_SUBTRACT: ret = KeyCode::eSubtract; break;
            case GLFW_KEY_KP_MULTIPLY: ret = KeyCode::eMultiply; break;
            case GLFW_KEY_KP_DIVIDE: ret = KeyCode::eDivide; break;
            case GLFW_KEY_INSERT: ret = KeyCode::eInsert; break;
            case GLFW_KEY_DELETE: ret = KeyCode::eDelete; break;
            case GLFW_KEY_PAGE_UP: ret = KeyCode::ePageUp; break;
            case GLFW_KEY_PAGE_DOWN: ret = KeyCode::ePageDown; break;
            case GLFW_KEY_HOME: ret = KeyCode::eHome; break;
            case GLFW_KEY_END: ret = KeyCode::eEnd; break;
            case GLFW_KEY_SCROLL_LOCK: ret = KeyCode::eScrollLock; break;
            case GLFW_KEY_NUM_LOCK: ret = KeyCode::eNumLock; break;
            case GLFW_KEY_PRINT_SCREEN: ret = KeyCode::ePrintScreen; break;
            case GLFW_KEY_PAUSE: ret = KeyCode::ePause; break;
            case GLFW_KEY_MENU: ret = KeyCode::eMenu; break;

            default: break;
        }

        return ret;
    }

    using Input::MouseCode;
    static MouseCode ToVtMouseCode(i32 mouse)
    {
        MouseCode ret = MouseCode::eUnknown;
        switch (mouse)
        {
            case GLFW_MOUSE_BUTTON_LEFT: ret = MouseCode::eLeft; break;
            case GLFW_MOUSE_BUTTON_RIGHT: ret = MouseCode::eRight; break;
            case GLFW_MOUSE_BUTTON_MIDDLE: ret = MouseCode::eMiddle; break;
            case GLFW_MOUSE_BUTTON_4: ret = MouseCode::eX1; break;
            case GLFW_MOUSE_BUTTON_5: ret = MouseCode::eX2; break;
            case GLFW_MOUSE_BUTTON_6: ret = MouseCode::eX3; break;
            case GLFW_MOUSE_BUTTON_7: ret = MouseCode::eX4; break;
            case GLFW_MOUSE_BUTTON_8: ret = MouseCode::eX5; break;
        }

        return ret;
    }

    static KeyCode VtKeyCode(u32 keycode, bool extended = false)
    {
        switch (keycode)
        {
            case '0': return KeyCode::eNum0;
            case '1': return KeyCode::eNum1;
            case '2': return KeyCode::eNum2;
            case '3': return KeyCode::eNum3;
            case '4': return KeyCode::eNum4;
            case '5': return KeyCode::eNum5;
            case '6': return KeyCode::eNum6;
            case '7': return KeyCode::eNum7;
            case '8': return KeyCode::eNum8;
            case '9': return KeyCode::eNum9;
            case 'A': return KeyCode::eA;
            case 'B': return KeyCode::eB;
            case 'C': return KeyCode::eC;
            case 'D': return KeyCode::eD;
            case 'E': return KeyCode::eE;
            case 'F': return KeyCode::eF;
            case 'G': return KeyCode::eG;
            case 'H': return KeyCode::eH;
            case 'I': return KeyCode::eI;
            case 'J': return KeyCode::eJ;
            case 'K': return KeyCode::eK;
            case 'L': return KeyCode::eL;
            case 'M': return KeyCode::eM;
            case 'N': return KeyCode::eN;
            case 'O': return KeyCode::eO;
            case 'P': return KeyCode::eP;
            case 'Q': return KeyCode::eQ;
            case 'R': return KeyCode::eR;
            case 'S': return KeyCode::eS;
            case 'T': return KeyCode::eT;
            case 'U': return KeyCode::eU;
            case 'V': return KeyCode::eV;
            case 'W': return KeyCode::eW;
            case 'X': return KeyCode::eX;
            case 'Y': return KeyCode::eY;
            case 'Z': return KeyCode::eZ;
            case VK_OEM_3: return KeyCode::eTilde;
            case VK_F1: return KeyCode::eF1;
            case VK_F2: return KeyCode::eF2;
            case VK_F3: return KeyCode::eF3;
            case VK_F4: return KeyCode::eF4;
            case VK_F5: return KeyCode::eF5;
            case VK_F6: return KeyCode::eF6;
            case VK_F7: return KeyCode::eF7;
            case VK_F8: return KeyCode::eF8;
            case VK_F9: return KeyCode::eF9;
            case VK_F10: return KeyCode::eF10;
            case VK_F11: return KeyCode::eF11;
            case VK_F12: return KeyCode::eF12;
            case VK_F13: return KeyCode::eF13;
            case VK_F14: return KeyCode::eF14;
            case VK_F15: return KeyCode::eF15;
            case VK_ESCAPE: return KeyCode::eEscape;
            case VK_BACK: return KeyCode::eBackspace;
            case VK_TAB: return KeyCode::eTab;
            case VK_CAPITAL: return KeyCode::eCapsLock;
            case VK_RETURN:
                return extended ? KeyCode::eEnter : KeyCode::eReturn;
            case VK_LSHIFT: return KeyCode::eLShift;
            case VK_RSHIFT: return KeyCode::eRShift;
            case VK_LCONTROL: return KeyCode::eLCtrl;
            case VK_RCONTROL: return KeyCode::eRCtrl;
            case VK_MENU: return extended ? KeyCode::eRAlt : KeyCode::eLAlt;
            case VK_LWIN: return KeyCode::eLSystem;
            case VK_RWIN: return KeyCode::eRSystem;
            case VK_SPACE: return KeyCode::eSpace;
            case VK_OEM_MINUS: return KeyCode::eHyphen;
            case VK_OEM_PLUS: return KeyCode::eEqual;
            case VK_DECIMAL: return KeyCode::eDecimal;
            case VK_OEM_4: return KeyCode::eLBracket;
            case VK_OEM_6: return KeyCode::eRBracket;
            case VK_OEM_1: return KeyCode::eSemicolon;
            case VK_OEM_7: return KeyCode::eApostrophe;
            case VK_OEM_COMMA: return KeyCode::eComma;
            case VK_OEM_PERIOD: return KeyCode::ePeriod;
            case VK_OEM_2: return KeyCode::eSlash;
            case VK_OEM_5: return KeyCode::eBackslash;
            case VK_UP: return KeyCode::eUp;
            case VK_DOWN: return KeyCode::eDown;
            case VK_LEFT: return KeyCode::eLeft;
            case VK_RIGHT: return KeyCode::eRight;
            case VK_NUMPAD0: return KeyCode::eNumpad0;
            case VK_NUMPAD1: return KeyCode::eNumpad1;
            case VK_NUMPAD2: return KeyCode::eNumpad2;
            case VK_NUMPAD3: return KeyCode::eNumpad3;
            case VK_NUMPAD4: return KeyCode::eNumpad4;
            case VK_NUMPAD5: return KeyCode::eNumpad5;
            case VK_NUMPAD6: return KeyCode::eNumpad6;
            case VK_NUMPAD7: return KeyCode::eNumpad7;
            case VK_NUMPAD8: return KeyCode::eNumpad8;
            case VK_NUMPAD9: return KeyCode::eNumpad9;
            case VK_SEPARATOR: return KeyCode::eSeparator;
            case VK_ADD: return KeyCode::eAdd;
            case VK_SUBTRACT: return KeyCode::eSubtract;
            case VK_MULTIPLY: return KeyCode::eMultiply;
            case VK_DIVIDE: return KeyCode::eDivide;
            case VK_INSERT: return KeyCode::eInsert;
            case VK_DELETE: return KeyCode::eDelete;
            case VK_PRIOR: return KeyCode::ePageUp;
            case VK_NEXT: return KeyCode::ePageDown;
            case VK_HOME: return KeyCode::eHome;
            case VK_END: return KeyCode::eEnd;
        }

        return KeyCode::eUnknown;
    }
    static MouseCode VtMouseCode(u32 mousecode)
    {
        switch (mousecode)
        {
            case MK_LBUTTON: return MouseCode::eLeft;
            case MK_MBUTTON: return MouseCode::eMiddle;
            case MK_RBUTTON: return MouseCode::eRight;
            case MK_XBUTTON1: return MouseCode::eX1;
            case MK_XBUTTON2: return MouseCode::eX2;
        }

        return MouseCode::eUnknown;
    }

    Win32Window::Win32Window(const WindowSpecification& specification)
        : Window(specification)
    {
        i32                           width   = specification.VideoMode.Width;
        i32                           height  = specification.VideoMode.Height;
        const char*                   title   = specification.Title.data();
        auto                          style   = GetWindowStyle();
        auto                          styleEx = GetWindowStyleEx();

        Ref<Monitor>                  monitor = specification.Monitor;
        [[maybe_unused]] GLFWmonitor* monitorHandle
            = monitor ? std::any_cast<GLFWmonitor*>(monitor->GetNativeHandle())
                      : nullptr;

        RECT wrect{};
        wrect.left   = 0;
        wrect.right  = specification.VideoMode.Width;
        wrect.top    = 0;
        wrect.bottom = specification.VideoMode.Height;
        AdjustWindowRectEx(&wrect, style, false, 0);
        HDC dc                  = GetDC(nullptr);

        m_Data.VideoMode.Width  = wrect.right - wrect.left;
        m_Data.VideoMode.Height = wrect.bottom - wrect.top;
        m_Data.Position         = {wrect.left, wrect.top};
        i32 x                   = (GetDeviceCaps(dc, HORZRES)
                 - static_cast<i32>(specification.VideoMode.Width))
              / 2;
        i32 y = (GetDeviceCaps(dc, VERTRES)
                 - static_cast<i32>(specification.VideoMode.Height))
              / 2;

        std::wstring wideTitle;
        Utf32::ToWide(specification.Title.begin(), specification.Title.end(),
                      std::back_inserter(wideTitle), 0);

        HINSTANCE hInstance = GetModuleHandleW(nullptr);
        m_WindowHandle      = CreateWindowExW(
            styleEx, s_WindowClassName, wideTitle.data(), style, x, y, width,
            height, nullptr, nullptr, hInstance, nullptr);
        WinAssert(m_WindowHandle);

        if (monitor)
        {
            [[maybe_unused]] std::string_view monitorName = monitor->GetName();
            VideoMode                         currentMode
                = specification.Monitor->GetCurrentVideoMode();
            [[maybe_unused]] u32 bitsPerPixel = currentMode.RedBits
                                              + currentMode.GreenBits
                                              + currentMode.BlueBits;
            [[maybe_unused]] u32 redBits     = currentMode.RedBits;
            [[maybe_unused]] u32 greenBits   = currentMode.GreenBits;
            [[maybe_unused]] i32 blueBits    = currentMode.BlueBits;
            [[maybe_unused]] u32 refreshRate = currentMode.RefreshRate;

            VtCoreTrace(
                "GLFW: Using monitor: {{ name: {}, currentMode: '{} x {} x {} "
                "({} "
                "{} {}) {} Hz'}}",
                monitorName, currentMode.Width, currentMode.Height,
                bitsPerPixel, redBits, greenBits, blueBits, refreshRate);
        }
        VtCoreTrace(
            "Win32: Created window {{ width: {}, height: {}, title: {} }}",
            width, height, title);

        ++s_WindowsCount;
        SetVisible(true);
        m_Data.IsOpen = true;

        SetupEvents();
        GetWindowMap()[m_WindowHandle] = this;

        if (!specification.NoAPI)
            m_SwapChain = SwapChain::Create(this, specification.VSync);

        ShowWindow(m_WindowHandle, SW_SHOW);
        UpdateWindow(m_WindowHandle);
    }
    Win32Window::~Win32Window()
    {
        // glfwDestroyWindow(m_Window);
        GetWindowMap().erase(m_WindowHandle);
        DestroyWindow(m_WindowHandle);

        if (m_BigIcon) DestroyIcon(m_BigIcon);
        if (m_SmallIcon) DestroyIcon(m_SmallIcon);

        --s_WindowsCount;
    }

    void Win32Window::PollEvents()
    {

        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // glfwPollEvents();
    }
    void Win32Window::Present() { m_SwapChain->Present(); }

    bool Win32Window::IsFocused() const noexcept
    {
        return GetActiveWindow() == m_WindowHandle;
    }
    bool Win32Window::IsMinimized() const noexcept
    {
        return IsIconic(m_WindowHandle);
    }
    bool Win32Window::IsHovered() const noexcept
    {
        // TODO(v1tr10l7): Win32: is Hovered return
        return m_Data.MouseHovered;
        // return glfwGetWindowAttrib(m_Window, GLFW_HOVERED);
    }
    std::string Win32Window::GetTitle() const noexcept { return m_Data.Title; }
    Vec2i       Win32Window::GetPosition() const noexcept
    {
        POINT pos = {0, 0};
        ClientToScreen(m_WindowHandle, &pos);

        return Vec2i(pos.x, pos.y);
    }
    Vec2i Win32Window::GetSize() const noexcept
    {
        RECT area;
        GetClientRect(m_WindowHandle, &area);

        return Vec2i(area.right, area.bottom);
    }
    Vec2i Win32Window::GetFramebufferSize() const noexcept { return GetSize(); }
    Vec2f Win32Window::GetContentScale() const noexcept
    {
        Vec2f          ret;
        const HMONITOR monitor
            = MonitorFromWindow(m_WindowHandle, MONITOR_DEFAULTTONEAREST);

        UINT xdpi, ydpi;
        if (GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi) != S_OK)
        {
            VtCoreError("Win32: Failed to query monitor DPI");
            return {0, 0};
        }

        ret.x = xdpi / (float)USER_DEFAULT_SCREEN_DPI;
        ret.y = ydpi / (float)USER_DEFAULT_SCREEN_DPI;

        return ret;
    }
    f32 Win32Window::GetOpacity() const noexcept
    {
        BYTE  alpha = 0;
        DWORD flags = 0;

        if ((GetWindowLongW(m_WindowHandle, GWL_EXSTYLE) & WS_EX_LAYERED)
            && GetLayeredWindowAttributes(m_WindowHandle, NULL, &alpha, &flags)
            && (flags & LWA_ALPHA))
            return alpha / 255.0f;

        return 1.0f;
    }

    void Win32Window::Close() noexcept
    {
        m_Data.IsOpen = false;
        WindowEvents::WindowClosedEvent(this);
        // glfwWindowShouldClose(m_Window);
    }
    void Win32Window::RequestFocus() noexcept
    {
        BringWindowToTop(m_WindowHandle);
        SetForegroundWindow(m_WindowHandle);
        SetFocus(m_WindowHandle);
    }
    void Win32Window::RequestUserAttention() const noexcept
    {
        // TODO(v1tr10l7): Win32: Request user attention
        // glfwRequestWindowAttention(m_Window);
    }
    void Win32Window::Maximize() noexcept
    {
        if (IsWindowVisible(m_WindowHandle))
            ShowWindow(m_WindowHandle, SW_MAXIMIZE);
    }
    void Win32Window::Minimize() noexcept
    {
        ShowWindow(m_WindowHandle, SW_MINIMIZE);
    }
    void Win32Window::Restore() noexcept
    {
        ShowWindow(m_WindowHandle, SW_RESTORE);
    }
    void Win32Window::SetTitle(std::string_view title)
    {
        m_Data.Title = title;

        std::wstring wideTitle;
        wideTitle.reserve(title.length() + 1);
        Utf32::ToWide(title.begin(), title.end(), std::back_inserter(wideTitle),
                      0);

        SetWindowTextW(m_WindowHandle, wideTitle.c_str());
    }
    void Win32Window::SetIcon(const Icon* icons, usize count)
    {
        VtCoreAssert(count > 0);

        const Image& bigImage
            = ChooseImage(icons, count, GetSystemMetrics(SM_CXICON),
                          GetSystemMetrics(SM_CYICON));
        const Image& smallImage
            = ChooseImage(icons, count, GetSystemMetrics(SM_CXSMICON),
                          GetSystemMetrics(SM_CYSMICON));

        HICON bigIcon   = CreateIconOrCursor(bigImage, 0, 0);
        HICON smallIcon = CreateIconOrCursor(smallImage, 0, 0);

        SendMessageW(m_WindowHandle, WM_SETICON, ICON_BIG,
                     reinterpret_cast<LPARAM>(bigIcon));
        SendMessageW(m_WindowHandle, WM_SETICON, ICON_SMALL,
                     reinterpret_cast<LPARAM>(smallIcon));

        if (m_BigIcon) DestroyIcon(m_BigIcon);
        if (m_SmallIcon) DestroyIcon(m_SmallIcon);

        m_BigIcon   = bigIcon;
        m_SmallIcon = smallIcon;
    }
    void Win32Window::SetPosition(i32 x, i32 y)
    {
        RECT rect    = {x, y, x, y};
        LONG style   = GetWindowLongW(m_WindowHandle, GWL_STYLE);
        LONG exStyle = GetWindowLongW(m_WindowHandle, GWL_EXSTYLE);

        AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle,
                                 GetDpiForWindow(m_WindowHandle));

        // SWP_NOACTIVATE - Don't activate the window
        // SWP_NOZORDER - Retain the current Z order
        // SWP_NOSIZE - Don't change the size of window
        SetWindowPos(m_WindowHandle, nullptr, rect.left, rect.top, 0, 0,
                     SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
    }
    void Win32Window::SetAspectRatio(i32 numerator, i32 denominator)
    {
        if (numerator == 0 || denominator == 0
            || (numerator != -1 && numerator < 0)
            || (denominator != -1 && denominator < 0))
        {
            VtCoreError("Win32: Invalid window aspect ratio: {}:{}", numerator,
                        denominator);
            return;
        }

        m_Data.Numererator = numerator;
        m_Data.Denominator = denominator;

        RECT area;
        GetWindowRect(m_WindowHandle, &area);
        ApplyAspectRatio(WMSZ_BOTTOMRIGHT, area);
        MoveWindow(m_WindowHandle, area.left, area.top, area.right - area.left,
                   area.bottom - area.top, TRUE);
    }
    void Win32Window::SetSize(const Vec2i& size) noexcept
    {
        RECT rect    = {0, 0, (LONG)size.x, (LONG)size.y};
        LONG style   = GetWindowLongW(m_WindowHandle, GWL_STYLE);
        LONG exStyle = GetWindowLongW(m_WindowHandle, GWL_EXSTYLE);

        // Client to Screen
        AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle,
                                 GetDpiForWindow(m_WindowHandle));

        SetWindowPos(m_WindowHandle, nullptr, 0, 0, rect.right - rect.left,
                     rect.bottom - rect.top,
                     SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }
    void Win32Window::SetOpacity(f32 opacity)
    {
        LONG exStyle = GetWindowLongW(m_WindowHandle, GWL_EXSTYLE);
        if (opacity < 1.f || (exStyle & WS_EX_TRANSPARENT))
        {
            const BYTE alpha = static_cast<BYTE>(255 * opacity);
            exStyle |= WS_EX_LAYERED;
            SetWindowLongW(m_WindowHandle, GWL_EXSTYLE, exStyle);
            SetLayeredWindowAttributes(m_WindowHandle, 0, alpha, LWA_ALPHA);
        }
        else if (exStyle & WS_EX_TRANSPARENT)
            SetLayeredWindowAttributes(m_WindowHandle, 0, 0, 0);
        else
        {
            exStyle &= ~WS_EX_LAYERED;
            SetWindowLongW(m_WindowHandle, GWL_EXSTYLE, exStyle);
        }
    }
    void Win32Window::SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                   i32 maxHeight)
    {
        if (minWidth != -1 && minHeight != -1
            && (minWidth < 0 || minHeight < 0))
        {
            VtCoreError("Win32: Invalid window minimum size: {}:{}", minWidth,
                        minHeight);
            return;
        }

        if (maxWidth != -1 && maxHeight != -1
            && (maxWidth < 0 || maxHeight < 0 || maxWidth < minWidth
                || maxHeight < minHeight))
        {
            VtCoreError("Win32: Invalid window maximum size: {}:{}", maxWidth,
                        maxHeight);
            return;
        }

        m_Data.MinWidth  = minWidth;
        m_Data.MinHeight = minHeight;
        m_Data.MaxWidth  = maxWidth;
        m_Data.MaxHeight = maxHeight;

        RECT area;
        GetWindowRect(m_WindowHandle, &area);
        MoveWindow(m_WindowHandle, area.left, area.top, area.right - area.left,
                   area.bottom - area.top, TRUE);
    }

    void Win32Window::SetAutoIconify(bool autoIconify) const noexcept
    {
        // TODO(v1tr10l7): Win32: SetAutoIconify
        (void)autoIconify;
    }
    void Win32Window::SetCursorPosition(Vec2d position) noexcept
    {
        POINT pos
            = {static_cast<i32>(position.x), static_cast<i32>(position.y)};

        m_LastCursorPos.x = pos.x;
        m_LastCursorPos.y = pos.y;

        ClientToScreen(m_WindowHandle, &pos);
        SetCursorPos(pos.x, pos.y);
    }
    void Win32Window::ShowCursor() const noexcept { ::ShowCursor(true); }
    void Win32Window::HideCursor() const noexcept { ::ShowCursor(false); }
    void Win32Window::SetFullscreen(bool fullscreen)
    {
        (void)fullscreen;
        VtCoreError(
            "Vortex currently doesn't support switching fullscreen on this "
            "platform.");
    }
    void Win32Window::SetResizable(bool resizable) noexcept
    {
        // m_Data.Resizable = resizable;
        // TODO(v1tr10l7): SetResizable
        (void)resizable;
    }
    void Win32Window::SetVisible(bool visible) const
    {
        ShowWindow(m_WindowHandle, visible ? SW_SHOW : SW_HIDE);
    }
    void Win32Window::SetAlwaysOnTop(bool alwaysOnTop)
    {
        // TODO(v1tr10l7): Win32: SetAlwaysOnTop
        (void)alwaysOnTop;
    }

    DWORD Win32Window::GetWindowStyle()
    {
        DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        if (!m_Data.Fullscreen) return style | WS_POPUP;

        style |= WS_SYSMENU | WS_MINIMIZEBOX;
        if (m_Data.Decorated)
        {
            style |= WS_CAPTION;
            if (m_Data.Resizable) style |= WS_MINIMIZEBOX | WS_THICKFRAME;
            return style | WS_OVERLAPPEDWINDOW;
        }

        return style | WS_POPUP;
    }
    DWORD Win32Window::GetWindowStyleEx()
    {
        DWORD styleEx = WS_EX_APPWINDOW;
        if (m_Data.Fullscreen || m_Data.AlwaysOnTop) styleEx |= WS_EX_TOPMOST;

        return styleEx;
    }
    void Win32Window::SetupEvents()
    {
        using namespace WindowEvents;
        return;

#define VtGetWindow(handle)                                                    \
    reinterpret_cast<Win32Window*>(glfwGetWindowUserPointer(handle))
#pragma region callbacks
        [[maybe_unused]] auto positionCallback
            = [](GLFWwindow* handle, i32 xpos, i32 ypos)
        {
            auto window = VtGetWindow(handle);
            WindowMovedEvent(window, xpos, ypos);

            window->m_Data.Position.x = xpos;
            window->m_Data.Position.y = ypos;
        };
        [[maybe_unused]] auto sizeCallback
            = [](GLFWwindow* handle, i32 width, i32 height)
        {
            auto window = VtGetWindow(handle);
            WindowResizedEvent(window, width, height);

            window->m_Data.VideoMode.Width  = width;
            window->m_Data.VideoMode.Height = height;
        };
        [[maybe_unused]] auto closeCallback = [](GLFWwindow* handle)
        {
            auto window = VtGetWindow(handle);
            WindowClosedEvent(window);

            window->m_Data.IsOpen = false;
        };
        [[maybe_unused]] auto focusCallback
            = [](GLFWwindow* handle, i32 focused)
        {
            auto window = VtGetWindow(handle);
            if (focused) WindowFocusedEvent(window);
            else WindowFocusLostEvent(window);

            window->m_Data.Focused = focused;
        };
        [[maybe_unused]] auto iconifyCallback
            = [](GLFWwindow* handle, i32 iconified)
        {
            auto window = VtGetWindow(handle);
            WindowMinimizedEvent(window, iconified);
        };
        [[maybe_unused]] auto maximizeCallback
            = [](GLFWwindow* handle, i32 maximized)
        {
            auto window = VtGetWindow(handle);
            WindowMaximizedEvent(window, maximized);
        };
        auto framebufferCallback = [](GLFWwindow* handle, i32 width, i32 height)
        {
            auto window = VtGetWindow(handle);
            if (window->m_SwapChain) window->m_SwapChain->OnResize();

            FramebufferResizedEvent(window, width, height);
        };
        [[maybe_unused]] auto keyCallback =
            [](GLFWwindow* handle, i32 key, i32 scancode, i32 action, i32 mods)
        {
            auto window = VtGetWindow(handle);
            using Input::KeyCode;
            KeyCode keycode = ToVtKeyCode(key);
            (void)scancode;
            (void)mods;

            switch (action)
            {
                case GLFW_PRESS:
                    KeyPressedEvent(window, keycode, 0);
                    window->m_Data.Keys[std::to_underlying(keycode)] = true;
                    break;
                case GLFW_RELEASE:
                    KeyReleasedEvent(window, keycode);
                    window->m_Data.Keys[std::to_underlying(keycode)] = false;
                    break;
                case GLFW_REPEAT: KeyPressedEvent(window, keycode, 1); break;
            }
        };
        [[maybe_unused]] auto charCallback
            = [](GLFWwindow* handle, u32 codepoint)
        {
            auto window = VtGetWindow(handle);
            KeyTypedEvent(window, codepoint);
        };
        [[maybe_unused]] auto mouseButtonCallback
            = [](GLFWwindow* handle, i32 button, i32 action, i32 mods)
        {
            auto window = VtGetWindow(handle);
            using Input::MouseCode;
            MouseCode mousecode = ToVtMouseCode(button);
            (void)mods;

            if (action == GLFW_PRESS)
            {
                MouseButtonPressedEvent(window, mousecode);
                window->m_Data.MouseButtons[std::to_underlying(mousecode)]
                    = true;
            }
            else if (action == GLFW_RELEASE)
            {
                MouseButtonReleasedEvent(window, mousecode);
                window->m_Data.MouseButtons[std::to_underlying(mousecode)]
                    = false;
            }
        };
        [[maybe_unused]] auto cursorEnterCallback
            = [](GLFWwindow* handle, i32 entered)
        {
            auto window = VtGetWindow(handle);
            if (entered) MouseEnteredEvent(window);
            else MouseLeftEvent(window);
        };
        [[maybe_unused]] auto scrollCallback
            = [](GLFWwindow* handle, f64 xoffset, f64 yoffset)
        {
            auto window = VtGetWindow(handle);
            MouseScrolledEvent(window, xoffset, yoffset);
        };
        [[maybe_unused]] auto cursorPosCallback
            = [](GLFWwindow* handle, f64 xpos, f64 ypos)
        {
            auto window = VtGetWindow(handle);
            MouseMovedEvent(window, xpos, ypos);
            window->m_Data.MousePosition = {xpos, ypos};
        };
        auto charModsCallback = [](GLFWwindow* handle, u32 codepoint, i32 mods)
        {
            auto window = VtGetWindow(handle);
            using Input::KeyCode;
            KeyCode keycode = ToVtKeyCode(mods);
            UnicodeKeyTypedEvent(window, codepoint, keycode);
        };
        auto dropCallback
            = [](GLFWwindow* handle, i32 pathCount, const char** paths)
        {
            auto window = VtGetWindow(handle);
            for (i32 i = 0; i < pathCount; i++)
                FileDroppedEvent(window, paths[i]);
        };
        auto joystickCallback = [](i32 jid, i32 event)
        {
            if (event == GLFW_CONNECTED) GamepadConnectedEvent(jid);
            else if (event == GLFW_DISCONNECTED) GamepadDisconnectedEvent(jid);
        };
        // TODO(v1tr10l7): Set cursor
#pragma endregion

#undef VtGetWindow
        glfwSetWindowPosCallback(m_Window, positionCallback);
        glfwSetWindowSizeCallback(m_Window, sizeCallback);
        glfwSetWindowCloseCallback(m_Window, closeCallback);
        glfwSetWindowFocusCallback(m_Window, focusCallback);
        glfwSetWindowIconifyCallback(m_Window, iconifyCallback);
        glfwSetWindowMaximizeCallback(m_Window, maximizeCallback);
        glfwSetFramebufferSizeCallback(m_Window, framebufferCallback);
        glfwSetKeyCallback(m_Window, keyCallback);
        glfwSetCharCallback(m_Window, charCallback);
        glfwSetMouseButtonCallback(m_Window, mouseButtonCallback);
        glfwSetCursorEnterCallback(m_Window, cursorEnterCallback);
        glfwSetScrollCallback(m_Window, scrollCallback);
        glfwSetCursorPosCallback(m_Window, cursorPosCallback);
        glfwSetCharModsCallback(m_Window, charModsCallback);
        glfwSetDropCallback(m_Window, dropCallback);
        glfwSetJoystickCallback(joystickCallback);
    }
    void Win32Window::ApplyAspectRatio(i32 edge, RECT& area)
    {
        RECT      frame   = {};
        LONG      style   = GetWindowLongW(m_WindowHandle, GWL_STYLE);
        LONG      exStyle = GetWindowLongW(m_WindowHandle, GWL_EXSTYLE);
        const f32 ratio   = static_cast<f32>(m_Data.Numererator)
                        / static_cast<f32>(m_Data.Denominator);

        AdjustWindowRectExForDpi(&frame, style, FALSE, exStyle,
                                 GetDpiForWindow(m_WindowHandle));

        if (edge == WMSZ_LEFT || edge == WMSZ_BOTTOMLEFT || edge == WMSZ_RIGHT
            || edge == WMSZ_BOTTOMRIGHT)
            area.bottom = area.top + (frame.bottom - frame.top)
                        + static_cast<i32>(((area.right - area.left)
                                            - (frame.right - frame.left))
                                           / ratio);
        else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
            area.top = area.bottom - (frame.bottom - frame.top)
                     - static_cast<i32>(((area.right - area.left)
                                         - (frame.right - frame.left))
                                        / ratio);
        else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
            area.right = area.left + (frame.right - frame.left)
                       + static_cast<i32>(((area.bottom - area.top)
                                           - (frame.bottom - frame.top))
                                          * ratio);
    }

    LRESULT WINAPI Win32Window::HandleEvents(HWND hWnd, UINT msg, WPARAM wParam,
                                             LPARAM lParam)
    {
        switch (msg)
        {
            case WM_MOVE:
                WindowEvents::WindowMovedEvent(this, LOWORD(lParam),
                                               HIWORD(lParam));
                break;
            case WM_INPUT:
            {
                break;
            }
            case WM_ACTIVATEAPP: break;
            case WM_CHAR:
            case WM_SYSCHAR:
            {
                u32 highSurrogate = 0;
                if (wParam >= 0xd800 && wParam <= 0xdbff)
                    highSurrogate = static_cast<WCHAR>(wParam);
                else
                {
                    u32 codepoint = 0;

                    if (wParam >= 0xdc00 && wParam <= 0xdfff)
                    {
                        if (highSurrogate)
                        {
                            codepoint += (highSurrogate - 0xd800) << 10;
                            codepoint += static_cast<WCHAR>(wParam) - 0xdc00;
                            codepoint += 0x10000;
                        }
                    }
                    else codepoint = static_cast<WCHAR>(wParam);
                    WindowEvents::KeyTypedEvent(this, codepoint);
                }
                return 0;
            }
            case WM_UNICHAR:
            {
                if (wParam == UNICODE_NOCHAR) return TRUE;
                u32 c = static_cast<u32>(wParam);
                WindowEvents::KeyTypedEvent(this, c);
                return FALSE;
            }
            case WM_CLOSE:
            case WM_DESTROY:
                m_Data.IsOpen = false;
                // PostQuitMessage(0);
                break;
            case WM_DEVICECHANGE: break;
            case WM_DISPLAYCHANGE: break;
            case WM_DROPFILES: break;
            case WM_ERASEBKGND: return TRUE;
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                KeyCode key
                    = VtKeyCode(static_cast<u32>(wParam), lParam & (1 << 24));
                if (((u32)lParam & (1 << 31)) == 0)
                {
                    WindowEvents::KeyPressedEvent(this, key, lParam & 0xFF00);
                    m_Data.Keys[std::to_underlying(key)] = true;
                }
                else
                {
                    WindowEvents::KeyReleasedEvent(this, key);
                    m_Data.Keys[static_cast<u32>(key)] = false;
                }
                break;
            }
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
            {
                bool pressed = false;
                if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN
                    || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
                    pressed = true;

                MouseCode code = MouseCode::eUnknown;
                if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
                    code = MouseCode::eLeft;
                else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
                    code = MouseCode::eRight;
                else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
                    code = MouseCode::eMiddle;
                else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
                    code = MouseCode::eX1;
                else code = MouseCode::eX2;

                if (pressed) WindowEvents::MouseButtonPressedEvent(this, code);
                else WindowEvents::MouseButtonReleasedEvent(this, code);
                break;
            }

            case WM_LBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_XBUTTONDBLCLK:
                WindowEvents::MouseButtonPressedEvent(this,
                                                      VtMouseCode(wParam));
                break;
            case WM_MOUSEACTIVATE: break;
            case WM_NCMOUSELEAVE:
            case WM_MOUSELEAVE:
                if (m_CursorTracked)
                    WindowEvents::MouseMovedEvent(this, -FLT_MAX, FLT_MAX);

                m_CursorTracked     = false;
                m_Data.MouseHovered = false;
                WindowEvents::MouseLeftEvent(this);
                break;
            case WM_MOUSEMOVE:
            {
                if (!m_CursorTracked)
                {
                    TRACKMOUSEEVENT lpEventTrack = {};
                    lpEventTrack.cbSize          = sizeof(lpEventTrack);
                    lpEventTrack.dwFlags         = TIME_LEAVE;
                    lpEventTrack.hwndTrack       = m_WindowHandle;
                    lpEventTrack.dwHoverTime     = 0;
                    TrackMouseEvent(&lpEventTrack);

                    m_CursorTracked     = true;
                    m_Data.MouseHovered = true;
                    WindowEvents::MouseEnteredEvent(this);
                }

                POINT pos       = {(LONG)LOWORD(lParam), (LONG)HIWORD(lParam)};
                // if (msg == WM_MOUSEMOVE) ClientToScreen(m_WindowHandle,
                // &pos);
                m_Data.Position = {pos.x, pos.y};
                WindowEvents::MouseMovedEvent(this, pos.x, pos.y);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                i16 delta = HIWORD(wParam);
                WindowEvents::MouseScrolledEvent(this, delta / 120.0f, 0.0f);
                break;
            }
            case WM_MOUSEHWHEEL:
            {
                i16 delta = HIWORD(wParam);
                WindowEvents::MouseScrolledEvent(this, 0.0f, delta / 120.0f);
                break;
            }
            case WM_SIZE:
                m_Data.VideoMode.Width  = LOWORD(lParam);
                m_Data.VideoMode.Height = HIWORD(lParam);
                WindowEvents::WindowResizedEvent(this, LOWORD(lParam),
                                                 HIWORD(lParam));
                WindowEvents::FramebufferResizedEvent(this, LOWORD(lParam),
                                                      HIWORD(lParam));

                if (wParam == SIZE_MINIMIZED)
                    WindowEvents::WindowMinimizedEvent(this, true);
                break;
            case WM_SETFOCUS: WindowEvents::WindowFocusedEvent(this); break;
            case WM_KILLFOCUS: WindowEvents::WindowFocusLostEvent(this); break;
            case WM_SYSCOMMAND:
            {
                switch (wParam)
                {
                    case SC_MONITORPOWER:
                    {
                        MonitorState monitorState = {};
                        switch (lParam)
                        {
                            case -1:
                                monitorState = MonitorState::eConnected;
                                break;
                            case 2:
                                monitorState = MonitorState::eDisconnected;
                                break;
                            case 1:
                                monitorState = MonitorState::eLowPower;
                                break;
                        }

                        // TODO(v1tr10l7): Monitor state updated event
                        (void)monitorState;
                        break;
                    }
                    case SC_SCREENSAVE:
                        // NOTE: If fullscreen is enabled, disable screensaver
                        if (m_Data.Fullscreen) return 0;
                        // WindowEvents::monitorStateChangedEvent(
                        //     MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL),
                        //     MonitorState::ScreenSaver);
                        break;
                }
                break;
            }
            default: break;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT WINAPI Win32Window::HandleGlobalEvents(HWND hWnd, UINT msg,
                                                   WPARAM wParam, LPARAM lParam)
    {
        Win32Window* window = GetWindowMap()[hWnd];

        return window ? window->HandleEvents(hWnd, msg, wParam, lParam)
                      : DefWindowProc(hWnd, msg, wParam, lParam);
    }

    Image& Win32Window::ChooseImage(const Image* images, usize count, i32 width,
                                    i32 height)
    {
        i32    leastDiff = std::numeric_limits<i32>::max();
        Image& ret       = const_cast<Image&>(images[0]);

        for (auto& image : std::views::counted(images, count))
        {
            const i32 currentDiff = std::abs(
                image.GetWidth() * image.GetHeight() - width * height);

            if (currentDiff < leastDiff)
            {
                ret       = image;
                leastDiff = currentDiff;
            }
        }

        return ret;
    }

    HICON Win32Window::CreateIconOrCursor(const Image& image, i32 xhot,
                                          i32 yhot, bool icon)
    {
        BITMAPV5HEADER header{};
        header.bV5Size        = sizeof(header);
        header.bV5Width       = image.GetWidth();
        header.bV5Height      = -image.GetHeight();
        header.bV5Planes      = 1;
        header.bV5BitCount    = 32;
        header.bV5Compression = BI_BITFIELDS;
        header.bV5RedMask     = 0x00ff0000;
        header.bV5GreenMask   = 0x0000ff00;
        header.bV5BlueMask    = 0x000000ff;
        header.bV5AlphaMask   = 0xff000000;

        unsigned char* target = nullptr;
        HDC            dc     = GetDC(nullptr);
        HBITMAP        color  = CreateDIBSection(
            dc, reinterpret_cast<BITMAPINFO*>(&header), DIB_RGB_COLORS,
            reinterpret_cast<void**>(&target), nullptr, static_cast<DWORD>(0));
        ReleaseDC(nullptr, dc);

        if (!color)
        {
            VtCoreError("Win32: Failed to create RGBA bitmap");
            return nullptr;
        }

        HBITMAP mask
            = CreateBitmap(image.GetWidth(), image.GetHeight(), 1, 1, nullptr);
        if (!mask)
        {
            VtCoreError("Win32: Failed to create mask bitmap");
            DeleteObject(color);
            return nullptr;
        }

        unsigned char* source = image.GetPixels();
        for (i32 i = 0; i < image.GetWidth() * image.GetHeight(); i++)
        {
            target[0] = source[2];
            target[1] = source[1];
            target[2] = source[0];
            target[3] = source[3];
            target += 4;
            source += 4;
        }

        ICONINFO iconInfo{};
        iconInfo.fIcon    = icon;
        iconInfo.xHotspot = xhot;
        iconInfo.yHotspot = yhot;
        iconInfo.hbmMask  = mask;
        iconInfo.hbmColor = color;

        HICON handle      = CreateIconIndirect(&iconInfo);

        DeleteObject(color);
        DeleteObject(mask);

        if (!handle) VtCoreError("Win32: Failed to create image");

        return handle;
    }

    bool Win32Window::Initialize()
    {
        HINSTANCE   hInstance = GetModuleHandleW(nullptr);

        WNDCLASSEXW wcex{};
        wcex.cbSize      = sizeof(wcex);
        wcex.style       = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = HandleGlobalEvents;
        wcex.cbClsExtra  = 0;
        wcex.cbWndExtra  = 0;
        wcex.hInstance   = hInstance;
        wcex.hCursor
            = LoadCursorW(hInstance, reinterpret_cast<LPCWSTR>(IDC_ARROW));
        wcex.hIcon = reinterpret_cast<HICON>(
            LoadImageW(hInstance, L"Vortex_Icon", IMAGE_ICON, 0, 0,
                       LR_DEFAULTSIZE | LR_SHARED));
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName  = nullptr;
        wcex.lpszClassName = s_WindowClassName;
        wcex.hIconSm       = nullptr;

        WinAssert(RegisterClassExW(&wcex));

        glfwSetErrorCallback(glfwErrorCallback);
        return glfwInit() == GLFW_TRUE;
    }
    void Win32Window::Shutdown()
    {
        WinAssert(
            UnregisterClassW(s_WindowClassName, GetModuleHandleW(nullptr)));
        glfwTerminate();
    }
}; // namespace Vortex
