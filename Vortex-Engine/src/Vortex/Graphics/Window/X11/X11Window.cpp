/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Graphics/Window/X11/X11Window.hpp"

namespace Vortex
{
    usize       X11Window::s_WindowsCount = 0;

    static void glfwErrorCallback(int code, const char* description)
    {
        (void)code;
        (void)description;
        VtCoreError(
            "GLFW: An error has occured, ErrorCode: {}, Description: {}", code,
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

            default:
        }

        return ret;
    }

    using Input::MouseCode;
    MouseCode ToVtMouseCode(i32 mouse)
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

    X11Window::X11Window(const WindowSpecification& specification)
    {
        if (s_WindowsCount == 0)
        {
            VtCoreAssert(X11Window::Initialize());
            VtCoreInfo("GLFW: Successfully initialized, version: {}",
                       glfwGetVersionString());
        }

        i32         width  = specification.VideoMode.Width;
        i32         height = specification.VideoMode.Height;
        const char* title  = specification.Title.data();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, specification.Visible);
        m_Data.Visible = specification.Visible;
        glfwWindowHint(GLFW_DECORATED, specification.Decorated);
        m_Data.Decorated = specification.Decorated;
        glfwWindowHint(GLFW_FOCUSED, specification.Focused);
        m_Data.Focused = specification.Focused;
        glfwWindowHint(GLFW_FLOATING, specification.AlwaysOnTop);
        glfwWindowHint(GLFW_MAXIMIZED, specification.Maximized);
        glfwWindowHint(GLFW_POSITION_X, specification.Position.x);
        glfwWindowHint(GLFW_POSITION_Y, specification.Position.y);
        m_Data.Position = specification.Position;
        glfwWindowHint(GLFW_RED_BITS, specification.VideoMode.RedBits);
        glfwWindowHint(GLFW_GREEN_BITS, specification.VideoMode.GreenBits);
        glfwWindowHint(GLFW_BLUE_BITS, specification.VideoMode.BlueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, specification.VideoMode.RefreshRate);
        glfwWindowHint(GLFW_AUTO_ICONIFY, specification.AutoIconify);
        m_Data.AutoIconify = specification.AutoIconify;
        glfwWindowHint(GLFW_CENTER_CURSOR, specification.CenterCursor);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, specification.FocusOnShow);
        m_Data.FocusOnShow   = specification.FocusOnShow;

        Ref<Monitor> monitor = specification.Monitor;
        GLFWmonitor* monitorHandle
            = monitor ? std::any_cast<GLFWmonitor*>(monitor->GetNativeHandle())
                      : nullptr;

        m_Window = glfwCreateWindow(
            width, height, title,
            specification.Fullscreen ? monitorHandle : nullptr, nullptr);

        if (monitor)
        {
            [[maybe_unused]] std::string_view monitorName = monitor->GetName();
            VideoMode                         currentMode
                = specification.Monitor->GetCurrentVideoMode();
            [[maybe_unused]] u32 bitsPerPixel = currentMode.RedBits
                                              + currentMode.GreenBits
                                              + currentMode.BlueBits;
            [[maybe_unused]] u16 redBits     = currentMode.RedBits;
            [[maybe_unused]] u16 greenBits   = currentMode.GreenBits;
            [[maybe_unused]] u16 blueBits    = currentMode.BlueBits;
            [[maybe_unused]] u32 refreshRate = currentMode.RefreshRate;

            VtCoreTrace(
                "GLFW: Using monitor: {{ name: {}, currentMode: '{} x {} x {} "
                "({} "
                "{} {}) {} Hz'}}",
                monitorName, currentMode.Width, currentMode.Height,
                bitsPerPixel, redBits, greenBits, blueBits, refreshRate);
        }
        VtCoreTrace(
            "GLFW: Created window {{ width: {}, height: {}, title: {} }}",
            width, height, title);

        ++s_WindowsCount;
        SetVisible(true);
        m_Data.IsOpen = !glfwWindowShouldClose(m_Window);

        glfwSetWindowUserPointer(m_Window, reinterpret_cast<void*>(this));

        SetupEvents();

        m_Data.RendererContext = RendererContext::Create(this);
    }
    X11Window::~X11Window()
    {
        glfwDestroyWindow(m_Window);
        --s_WindowsCount;
        if (s_WindowsCount == 0) Shutdown();
    }

    void X11Window::PollEvents() { glfwPollEvents(); }
    void X11Window::Present() { m_Data.RendererContext->Present(); }

    bool X11Window::IsFocused() const noexcept { return m_Data.Focused; }
    bool X11Window::IsMinimized() const noexcept
    {
        return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
    }
    bool X11Window::IsHovered() const noexcept
    {
        return glfwGetWindowAttrib(m_Window, GLFW_HOVERED);
    }
    std::string X11Window::GetTitle() const noexcept
    {
        return glfwGetWindowTitle(m_Window);
    }
    Vec2i X11Window::GetPosition() const noexcept
    {
        Vec2i position;
        glfwGetWindowPos(m_Window, &position.x, &position.y);
        return position;
    }
    inline i32 X11Window::GetWidth() const noexcept
    {
        return m_Data.VideoMode.Width;
    }
    i32 X11Window::GetHeight() const noexcept
    {
        return m_Data.VideoMode.Height;
    }
    Vec2i X11Window::GetFramebufferSize() const noexcept
    {
        Vec2i ret;
        glfwGetFramebufferSize(m_Window, &ret.x, &ret.y);

        return ret;
    }
    Vec2f X11Window::GetContentScale() const noexcept
    {
        Vec2f ret;
        glfwGetWindowContentScale(m_Window, &ret.x, &ret.y);

        return ret;
    }
    f32 X11Window::GetOpacity() const noexcept
    {
        return glfwGetWindowOpacity(m_Window);
    }

    void X11Window::Close() noexcept
    {
        m_Data.IsOpen = false;
        glfwWindowShouldClose(m_Window);
    }
    void X11Window::RequestFocus() noexcept { glfwFocusWindow(m_Window); }
    void X11Window::RequestUserAttention() const noexcept
    {
        glfwRequestWindowAttention(m_Window);
    }
    void X11Window::Maximize() noexcept { glfwMaximizeWindow(m_Window); }
    void X11Window::Minimize() noexcept { glfwIconifyWindow(m_Window); }
    void X11Window::Restore() noexcept { glfwRestoreWindow(m_Window); }
    void X11Window::SetTitle(std::string_view title)
    {
        m_Data.Title = title;
        glfwSetWindowTitle(m_Window, title.data());
    }
    void X11Window::SetIcon(i32 count, const std::vector<Icon>& icons) const
    {
        std::vector<GLFWimage> images(count);
        for (auto& icon : icons)
        {
            GLFWimage image;
            image.width  = icon.Width;
            image.height = icon.Height;
            image.pixels = icon.Pixels;
            images.push_back(image);
        }

        glfwSetWindowIcon(m_Window, count, images.data());
    }
    void X11Window::SetPosition(i32 x, i32 y) const
    {
        glfwSetWindowPos(m_Window, x, y);
    }
    void X11Window::SetAspectRatio(i32 numerator, i32 denominator)
    {
        m_Data.Numererator = numerator;
        m_Data.Denominator = denominator;
        glfwSetWindowAspectRatio(m_Window, numerator, denominator);
    }
    void X11Window::SetWidth(const i32 width) noexcept
    {
        glfwSetWindowSize(m_Window, width, m_Data.VideoMode.Height);
    }
    void X11Window::SetHeight(const i32 height) noexcept
    {
        glfwSetWindowSize(m_Window, m_Data.VideoMode.Width, height);
    }
    void X11Window::SetOpacity(f32 opacity)
    {
        glfwSetWindowOpacity(m_Window, opacity);
    }
    void X11Window::SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                 i32 maxHeight)
    {
        m_Data.MinWidth  = minWidth;
        m_Data.MinHeight = minHeight;
        m_Data.MaxWidth  = maxWidth;
        m_Data.MaxHeight = maxHeight;
        glfwSetWindowSizeLimits(m_Window, minWidth, minHeight, maxWidth,
                                maxHeight);
    }

    void X11Window::SetAutoIconify(bool autoIconify) const noexcept
    {
        glfwSetWindowAttrib(m_Window, GLFW_AUTO_ICONIFY, autoIconify);
    }
    void X11Window::SetCursorPosition(Vec2d position) const noexcept
    {
        glfwSetCursorPos(m_Window, position.x, position.y);
    }
    void X11Window::ShowCursor() const noexcept
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    void X11Window::HideCursor() const noexcept
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    void X11Window::SetFullscreen(bool fullscreen)
    {
        (void)fullscreen;
        VtCoreError(
            "Vortex currently doesn't support switching fullscreen on this "
            "platform.");
    }
    void X11Window::SetResizable(bool resizable) noexcept
    {
        m_Data.Resizable = resizable;
        glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, resizable);
    }
    void X11Window::SetVisible(bool visible) const
    {
        if (visible) return glfwShowWindow(m_Window);
        glfwHideWindow(m_Window);
    }
    void X11Window::SetAlwaysOnTop(bool alwaysOnTop)
    {
        glfwSetWindowAttrib(m_Window, GLFW_FLOATING, alwaysOnTop);
    }

    void X11Window::SetupEvents()
    {
        using namespace WindowEvents;

#define VtGetWindow(handle)                                                    \
    reinterpret_cast<X11Window*>(glfwGetWindowUserPointer(handle))
#pragma region callbacks
        auto positionCallback = [](GLFWwindow* handle, i32 xpos, i32 ypos)
        {
            auto window = VtGetWindow(handle);
            WindowMovedEvent(window, xpos, ypos);

            window->m_Data.Position.x = xpos;
            window->m_Data.Position.y = ypos;
        };
        auto sizeCallback = [](GLFWwindow* handle, i32 width, i32 height)
        {
            auto window = VtGetWindow(handle);
            WindowResizedEvent(window, width, height);

            window->m_Data.VideoMode.Width  = width;
            window->m_Data.VideoMode.Height = height;
        };
        auto closeCallback = [](GLFWwindow* handle)
        {
            auto window = VtGetWindow(handle);
            WindowClosedEvent(window);

            window->m_Data.IsOpen = false;
        };
        auto focusCallback = [](GLFWwindow* handle, i32 focused)
        {
            auto window = VtGetWindow(handle);
            if (focused) WindowFocusedEvent(window);
            else WindowFocusLostEvent(window);

            window->m_Data.Focused = focused;
        };
        auto iconifyCallback = [](GLFWwindow* handle, i32 iconified)
        {
            auto window = VtGetWindow(handle);
            WindowMinimizedEvent(window, iconified);
        };
        auto maximizeCallback = [](GLFWwindow* handle, i32 maximized)
        {
            auto window = VtGetWindow(handle);
            WindowMaximizedEvent(window, maximized);
        };
        auto framebufferCallback = [](GLFWwindow* handle, i32 width, i32 height)
        {
            auto window = VtGetWindow(handle);
            window->m_Data.RendererContext->OnResize(static_cast<u32>(width),
                                                     static_cast<u32>(height));

            FramebufferResizedEvent(window, width, height);
        };
        auto keyCallback =
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
        auto charCallback = [](GLFWwindow* handle, u32 codepoint)
        {
            auto window = VtGetWindow(handle);
            KeyTypedEvent(window, codepoint);
        };
        auto mouseButtonCallback
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
        auto cursorEnterCallback = [](GLFWwindow* handle, i32 entered)
        {
            auto window = VtGetWindow(handle);
            if (entered) MouseEnteredEvent(window);
            else MouseLeavedEvent(window);
        };
        auto scrollCallback = [](GLFWwindow* handle, f64 xoffset, f64 yoffset)
        {
            auto window = VtGetWindow(handle);
            MouseScrolledEvent(window, xoffset, yoffset);
        };
        auto cursorPosCallback = [](GLFWwindow* handle, f64 xpos, f64 ypos)
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

    bool X11Window::Initialize()
    {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, GLFW_FALSE);

        glfwSetErrorCallback(glfwErrorCallback);

        return glfwInit() == GLFW_TRUE;
    }
    void X11Window::Shutdown() { glfwTerminate(); }
}; // namespace Vortex
