/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Renderer/Window/Wayland/WaylandWindow.hpp"

#include <xkbcommon/xkbcommon.h>

namespace Vortex
{
    wl_pointer_listener WaylandWindow::s_PointerListener = {
        .enter         = PointerHandleEnter,
        .leave         = PointerHandleLeave,
        .motion        = PointerHandleMotion,
        .button        = PointerHandleButton,
        .axis          = PointerHandleAxis,
        .frame         = PointerHandleFrame,
        .axis_source   = PointerHandleAxisSource,
        .axis_stop     = PointerHandleAxisStop,
        .axis_discrete = PointerHandleAxisDiscrete,
#if 0
        .axis_value120           = PointerHandleAxisValue120,
        .axis_relative_direction = PointerHandleAxisRelativeDirection,
#endif
    };
    wl_keyboard_listener WaylandWindow::s_KeyboardListener = {
        .keymap      = KeyboardHandleKeymap,
        .enter       = KeyboardHandleEnter,
        .leave       = KeyboardHandleLeave,
        .key         = KeyboardHandleKey,
        .modifiers   = KeyboardHandleModifiers,
        .repeat_info = KeyboardHandleRepeatInfo,
    };
    usize WaylandWindow::s_WindowsCount = 0;
    namespace
    {
        void RegistryHandleGlobal(void* userData, wl_registry* registry,
                                  u32 name, const char* interface, u32 version);
        void RegistryHandleGlobalRemove(void* userData, wl_registry* registry,
                                        u32 name);

        void SeatHandleCapabilities(void* userData, wl_seat* seat,
                                    u32 capabilities);
        void SeatHandleName(void* userData, wl_seat* seat, const char* name);

        wl_display*          s_Display          = nullptr;
        wl_registry*         s_Registry         = nullptr;
        wl_registry_listener s_RegistryListener = {
            .global        = RegistryHandleGlobal,
            .global_remove = RegistryHandleGlobalRemove,
        };
        const char*            s_ProxyTag      = glfwGetVersionString();
        xkb_context*           s_XkbContext    = nullptr;
        wl_compositor*         s_Compositor    = nullptr;
        wl_subcompositor*      s_Subcompositor = nullptr;
        wl_shm*                s_Shm           = nullptr;
        wl_seat*               s_Seat          = nullptr;
        const wl_seat_listener s_SeatListener  = {
             .capabilities = SeatHandleCapabilities,
             .name         = SeatHandleName,
        };
        // Mouse pointer
        wl_pointer*    s_Pointer            = nullptr;
        wl_keyboard*   s_Keyboard           = nullptr;
        // wl_touch*            s_Touch            = nullptr;
        //  TODO(v1tr10l7): touchpad events
        //  wl_touch_listener    s_TouchListener{};
        u32            s_Serial             = 0;
        u32            s_PointerEnterSerial = 0;
        WaylandWindow* s_FocusedWindow      = nullptr;

        void RegistryHandleGlobal(void* userData, wl_registry* registry,
                                  u32 name, const char* interface, u32 version)
        {
            VT_UNUSED(userData);

            if (std::strcmp(interface, "wl_compositor") == 0)
                s_Compositor
                    = reinterpret_cast<wl_compositor*>(wl_registry_bind(
                        registry, name, &wl_compositor_interface, 1));
            else if (std::strcmp(interface, "wl_subcompositor") == 0)
                s_Subcompositor
                    = reinterpret_cast<wl_subcompositor*>(wl_registry_bind(
                        registry, name, &wl_subcompositor_interface, 1));
            else if (std::strcmp(interface, "wl_shm") == 0)
                s_Shm = reinterpret_cast<wl_shm*>(
                    wl_registry_bind(registry, name, &wl_shm_interface, 1));
            else if (std::strcmp(interface, "wl_output") == 0)
            {
                ; // TODO(v1tr10l7): detected new monitor, we should handle that
                VtCoreInfo("Wayland: Detected monitor: {}", name);
            }
            else if (std::strcmp(interface, "wl_seat") == 0 && !s_Seat)
            {
                s_Seat = reinterpret_cast<wl_seat*>(wl_registry_bind(
                    registry, name, &wl_seat_interface, version));
                wl_seat_add_listener(s_Seat, &s_SeatListener, nullptr);

                if (wl_seat_get_version(s_Seat)
                    >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
                    VtCoreWarn("WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION: {}",
                               WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION);
            }
            else if (std::strcmp(interface, "wl_data_device_manager") == 0)
                ;
            else if (std::strcmp(interface, "xdg_wm_base") == 0)
                ;
            else if (std::strcmp(interface, "zxdg_decoration_manager_v1") == 0)
                ;
            else if (std::strcmp(interface, "wp_viewporter") == 0)
                ;
            else if (std::strcmp(interface, "zwp_relative_pointer_manager_v1")
                     == 0)
                ;
            else if (std::strcmp(interface, "zwp_pointer_constraints_v1") == 0)
                ;
            else if (std::strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
                ;
            else if (std::strcmp(interface, "xdg_activation_v1") == 0)
                ;
            else if (std::strcmp(interface, "wp_fractional_scale_manager_v1")
                     == 0)
                ;
            VtCoreTrace("Wayland: Registry global: {}", interface);
        }
        void RegistryHandleGlobalRemove(void* userData, wl_registry* registry,
                                        u32 name)
        {
            VT_UNUSED(userData);
            VT_UNUSED(registry);
            VT_UNUSED(name);
            [[maybe_unused]] std::source_location source
                = std::source_location::current();
            VtCoreWarn("Wayland: {} is not implemented!",
                       source.function_name());
        }

        void SeatHandleCapabilities(void* userData, wl_seat* seat,
                                    u32 capabilities)
        {
            VT_UNUSED(userData);

            if (capabilities & WL_SEAT_CAPABILITY_POINTER && !s_Pointer)
            {
                s_Pointer = wl_seat_get_pointer(seat);

                wl_pointer_add_listener(
                    s_Pointer, &WaylandWindow::s_PointerListener, nullptr);
            }
            else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && s_Pointer)
            {
                wl_pointer_destroy(s_Pointer);
                s_Pointer = nullptr;
            }

            if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !s_Keyboard)
            {
                s_Keyboard = wl_seat_get_keyboard(seat);
                wl_keyboard_add_listener(
                    s_Keyboard, &WaylandWindow::s_KeyboardListener, nullptr);
            }
            else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
                     && s_Keyboard)
            {
                wl_keyboard_destroy(s_Keyboard);
                s_Keyboard = nullptr;
            }

            // if (capabilities & WL_SEAT_CAPABILITY_TOUCH)
            //     ;
        }
        void SeatHandleName(void*, wl_seat*, const char* name)
        {
            VT_UNUSED(name);
            VtCoreInfo("Wayland: Bound to seat: '{}'", name);
        }
    }; // namespace

    static void glfwErrorCallback(int code, const char* description)
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

    WaylandWindow::WaylandWindow(const WindowSpecification& specification)
        : Window(specification)
    {
        if (s_WindowsCount == 0)
        {
            VtCoreAssert(WaylandWindow::Initialize());
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
        m_WindowHandle                 = glfwGetWaylandWindow(m_Window);
        GetWindowMap()[m_WindowHandle] = this;

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

        if (!specification.NoAPI)
            m_RendererContext
                = RendererContext::Create(this, specification.VSync);
    }
    WaylandWindow::~WaylandWindow()
    {
        VtCoreTrace("Wayland: Destroying window...");
        glfwDestroyWindow(m_Window);
        --s_WindowsCount;
        // if (s_WindowsCount == 0) Shutdown();
    }

    void WaylandWindow::PollEvents() { glfwPollEvents(); }
    void WaylandWindow::Present()
    {
        if (m_RendererContext) m_RendererContext->Present();
    }

    bool WaylandWindow::IsFocused() const noexcept { return m_Data.Focused; }
    bool WaylandWindow::IsMinimized() const noexcept
    {
        return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
    }
    bool WaylandWindow::IsHovered() const noexcept
    {
        return glfwGetWindowAttrib(m_Window, GLFW_HOVERED);
    }
    std::string WaylandWindow::GetTitle() const noexcept
    {
        return glfwGetWindowTitle(m_Window);
    }
    Vec2i WaylandWindow::GetPosition() const noexcept
    {
        // NOTE(v1tr10l7): wayland doesn't provide a way to retrieve window
        // position
        return {0, 0};
    }
    inline Vec2i WaylandWindow::GetSize() const noexcept
    {
        return Vec2i(m_Data.VideoMode.Width, m_Data.VideoMode.Height);
    }
    Vec2i WaylandWindow::GetFramebufferSize() const noexcept
    {
        Vec2i ret;
        glfwGetFramebufferSize(m_Window, &ret.x, &ret.y);

        return ret;
    }
    Vec2f WaylandWindow::GetContentScale() const noexcept
    {
        Vec2f ret;
        glfwGetWindowContentScale(m_Window, &ret.x, &ret.y);

        return ret;
    }
    f32  WaylandWindow::GetOpacity() const noexcept { return 1.0f; }

    void WaylandWindow::Close() noexcept
    {
        m_Data.IsOpen = false;
        glfwWindowShouldClose(m_Window);
    }
    void WaylandWindow::RequestFocus() noexcept { glfwFocusWindow(m_Window); }
    void WaylandWindow::RequestUserAttention() const noexcept
    {
        glfwRequestWindowAttention(m_Window);
    }
    void WaylandWindow::Maximize() noexcept { glfwMaximizeWindow(m_Window); }
    void WaylandWindow::Minimize() noexcept { glfwIconifyWindow(m_Window); }
    void WaylandWindow::Restore() noexcept { glfwRestoreWindow(m_Window); }
    void WaylandWindow::SetTitle(std::string_view title)
    {
        m_Data.Title = title;
        glfwSetWindowTitle(m_Window, title.data());
    }
    void WaylandWindow::SetIcon(const Icon* icons, usize count)
    {
        std::vector<GLFWimage> images;
        images.reserve(count);
        for (auto& icon : std::views::counted(icons, count))
        {
            GLFWimage image;
            image.width  = icon.GetWidth();
            image.height = icon.GetHeight();
            image.pixels = icon.GetPixels();
            images.push_back(image);
        }

        glfwSetWindowIcon(m_Window, count, images.data());
    }
    void WaylandWindow::SetPosition(i32 x, i32 y)
    {
        VT_UNUSED(x);
        VT_UNUSED(y);
        VtCoreWarn(
            "Wayland: The platform does not support setting the window "
            "position");
    }
    void WaylandWindow::SetAspectRatio(i32 numerator, i32 denominator)
    {
        m_Data.Numererator = numerator;
        m_Data.Denominator = denominator;
        glfwSetWindowAspectRatio(m_Window, numerator, denominator);
    }
    void WaylandWindow::SetSize(const Vec2i& size) noexcept
    {
        glfwSetWindowSize(m_Window, size.x, size.y);
    }
    void WaylandWindow::SetOpacity(f32 opacity)
    {
        VT_UNUSED(opacity);
        VtCoreWarn(
            "Wayland: The platform does not support setting the window "
            "opacity");
    }
    void WaylandWindow::SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                     i32 maxHeight)
    {
        m_Data.MinWidth  = minWidth;
        m_Data.MinHeight = minHeight;
        m_Data.MaxWidth  = maxWidth;
        m_Data.MaxHeight = maxHeight;
        glfwSetWindowSizeLimits(m_Window, minWidth, minHeight, maxWidth,
                                maxHeight);
    }

    void WaylandWindow::SetAutoIconify(bool autoIconify) const noexcept
    {
        glfwSetWindowAttrib(m_Window, GLFW_AUTO_ICONIFY, autoIconify);
    }
    void WaylandWindow::SetCursorPosition(Vec2d position) noexcept
    {
        VT_UNUSED(position);
        VtCoreWarn(
            "Wayland: The platform does not support setting cursor position");
    }
    void WaylandWindow::ShowCursor() const noexcept
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    void WaylandWindow::HideCursor() const noexcept
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    void WaylandWindow::SetFullscreen(bool fullscreen)
    {
        (void)fullscreen;
        VtCoreError(
            "Vortex currently doesn't support switching fullscreen on this "
            "platform.");
    }
    void WaylandWindow::SetResizable(bool resizable) noexcept
    {
        m_Data.Resizable = resizable;
        glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, resizable);
    }
    void WaylandWindow::SetVisible(bool visible) const
    {
        if (visible) return glfwShowWindow(m_Window);
        glfwHideWindow(m_Window);
    }
    void WaylandWindow::SetAlwaysOnTop(bool alwaysOnTop)
    {
        VT_UNUSED(alwaysOnTop);
        VtCoreWarn(
            "Wayland: The platform does not support making a window floating");
    }

    void WaylandWindow::SetupEvents()
    {
        using namespace WindowEvents;

#define VtGetWindow(handle)                                                    \
    reinterpret_cast<WaylandWindow*>(glfwGetWindowUserPointer(handle))
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

            if (window->m_RendererContext)
                window->m_RendererContext->OnResize();

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
        glfwSetCharModsCallback(m_Window, charModsCallback);
        glfwSetDropCallback(m_Window, dropCallback);
        glfwSetJoystickCallback(joystickCallback);
    }

    bool WaylandWindow::Initialize()
    {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

        glfwSetErrorCallback(glfwErrorCallback);

        bool status = glfwInit() == GLFW_TRUE;

        s_Display   = glfwGetWaylandDisplay();
        VtCoreAssertMsg(s_Display, "Wayland: Failed to open display");
        s_Registry                      = wl_display_get_registry(s_Display);
        s_PointerListener.enter         = PointerHandleEnter;
        s_PointerListener.leave         = PointerHandleLeave;
        s_PointerListener.motion        = PointerHandleMotion;
        s_PointerListener.button        = PointerHandleButton;
        s_PointerListener.axis          = PointerHandleAxis;
        s_PointerListener.frame         = PointerHandleFrame;
        s_PointerListener.axis_source   = PointerHandleAxisSource;
        s_PointerListener.axis_stop     = PointerHandleAxisStop;
        s_PointerListener.axis_discrete = PointerHandleAxisDiscrete;

        wl_registry_add_listener(s_Registry, &s_RegistryListener, nullptr);
        s_XkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        VtCoreAssertMsg(s_XkbContext,
                        "Wayland: Failed to initialize xkb context");

        // Sync so we got all registry objects
        wl_display_roundtrip(s_Display);

        // Sync so we got all initial output events
        wl_display_roundtrip(s_Display);

        VtCoreAssertMsg(s_Shm, "Failed to find wl_shm in the compositor");

        return status;
    }
    void WaylandWindow::Shutdown()
    {
#define VtRelease(object, destroy_function)                                    \
    if (object) destroy_function(object)

        VtRelease(s_XkbContext, xkb_context_unref);
        VtRelease(s_Subcompositor, wl_subcompositor_destroy);
        VtRelease(s_Compositor, wl_compositor_destroy);
        VtRelease(s_Shm, wl_shm_destroy);
        VtRelease(s_Pointer, wl_pointer_destroy);
        VtRelease(s_Keyboard, wl_keyboard_destroy);
        VtRelease(s_Seat, wl_seat_destroy);
        VtRelease(s_Registry, wl_registry_destroy);

        glfwTerminate();
    }

    void WaylandWindow::PointerHandleEnter(void* userData, wl_pointer* pointer,
                                           u32 serial, wl_surface* surface,
                                           wl_fixed_t xOffset,
                                           wl_fixed_t yOffset)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(xOffset);
        VT_UNUSED(yOffset);

        if (!surface) return;

        WaylandWindow* window = WaylandWindow::GetWindowMap()[surface];
        s_Serial              = serial;
        s_PointerEnterSerial  = serial;
        s_FocusedWindow       = window;

        WindowEvents::MouseEnteredEvent(window);
    }
    void WaylandWindow::PointerHandleLeave(void* userData, wl_pointer* pointer,
                                           u32 serial, wl_surface* surface)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);

        if (!surface) return;

        WaylandWindow* window = WaylandWindow::GetWindowMap()[surface];
        s_Serial              = serial;
        s_FocusedWindow       = nullptr;
        WindowEvents::MouseLeftEvent(window);
    }
    void WaylandWindow::PointerHandleMotion(void* userData, wl_pointer* pointer,
                                            u32 time, wl_fixed_t xOffset,
                                            wl_fixed_t yOffset)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(time);

        f64            xpos   = wl_fixed_to_double(xOffset);
        f64            ypos   = wl_fixed_to_double(yOffset);

        WaylandWindow* window = s_FocusedWindow;
        if (window) window->m_Data.MousePosition = Vec2d(xpos, ypos);
        WindowEvents::MouseMovedEvent(window, xpos, ypos);
    }
    void WaylandWindow::PointerHandleButton(void* userData, wl_pointer* pointer,
                                            u32 serial, u32 time, u32 button,
                                            u32 state)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(serial);
        VT_UNUSED(time);
        VT_UNUSED(button);
        VT_UNUSED(state);
    }
    void WaylandWindow::PointerHandleAxis(void* userData, wl_pointer* pointer,
                                          u32 time, u32 axis, wl_fixed_t value)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(time);
        VT_UNUSED(axis);
        VT_UNUSED(value);

        auto window = s_FocusedWindow;
        if (!window) return;

        f64 delta = -wl_fixed_to_double(value) / 10.0;

        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
            WindowEvents::MouseScrolledEvent(window, delta, 0.0);
        else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
            WindowEvents::MouseScrolledEvent(window, 0.0, delta);
    }
    void WaylandWindow::PointerHandleFrame(void* userData, wl_pointer* pointer)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
    }
    void WaylandWindow::PointerHandleAxisSource(void*       userData,
                                                wl_pointer* pointer,
                                                u32         axisSource)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(axisSource);
    }
    void WaylandWindow::PointerHandleAxisStop(void*       userData,
                                              wl_pointer* pointer, u32 time,
                                              u32 axis)
    {

        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(time);
        VT_UNUSED(axis);
    }
    void WaylandWindow::PointerHandleAxisDiscrete(void*       userData,
                                                  wl_pointer* pointer, u32 axis,
                                                  i32 discrete)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(axis);
        VT_UNUSED(discrete);
    }
    void WaylandWindow::PointerHandleAxisValue120(void*       userData,
                                                  wl_pointer* pointer, u32 axis,
                                                  i32 value120)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(axis);
        VT_UNUSED(value120);
    }
    void WaylandWindow::PointerHandleAxisRelativeDirection(void*       userData,
                                                           wl_pointer* pointer,
                                                           u32         axis,
                                                           u32 direction)
    {
        VT_UNUSED(userData);
        VT_UNUSED(pointer);
        VT_UNUSED(axis);
        VT_UNUSED(direction);
    }

    void WaylandWindow::KeyboardHandleKeymap(void*        userData,
                                             wl_keyboard* keyboard, u32 format,
                                             int fd, u32 size)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(format);
        VT_UNUSED(fd);
        VT_UNUSED(size);
    }
    void WaylandWindow::KeyboardHandleEnter(void*        userData,
                                            wl_keyboard* keyboard, u32 serial,
                                            wl_surface* surface, wl_array* keys)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(serial);
        VT_UNUSED(surface);
        VT_UNUSED(keys);
    }
    void WaylandWindow::KeyboardHandleLeave(void*        userData,
                                            wl_keyboard* keyboard, u32 serial,
                                            wl_surface* surface)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(serial);
        VT_UNUSED(surface);
    }
    void WaylandWindow::KeyboardHandleKey(void* userData, wl_keyboard* keyboard,
                                          u32 serial, u32 time, u32 scancode,
                                          u32 state)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(serial);
        VT_UNUSED(time);
        VT_UNUSED(scancode);
        VT_UNUSED(state);
    }
    void WaylandWindow::KeyboardHandleModifiers(void*        userData,
                                                wl_keyboard* keyboard,
                                                u32 serial, u32 modsDepressed,
                                                u32 modsLatched, u32 modsLocked,
                                                u32 group)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(serial);
        VT_UNUSED(modsDepressed);
        VT_UNUSED(modsLatched);
        VT_UNUSED(modsLocked);
        VT_UNUSED(group);
    }
    void WaylandWindow::KeyboardHandleRepeatInfo(void*        userData,
                                                 wl_keyboard* keyboard,
                                                 i32 rate, i32 delay)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(rate);
        VT_UNUSED(delay);
    }
}; // namespace Vortex
