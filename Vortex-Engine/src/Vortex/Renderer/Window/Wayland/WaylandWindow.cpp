/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Renderer/Window/Wayland/WaylandWindow.hpp"

#include <linux/input-event-codes.h>
#include <poll.h>

namespace Vortex
{
    usize WaylandWindow::s_WindowsCount = 0;
    namespace
    {
        [[maybe_unused]] const char* s_ProxyTag           = "Proxy tag";

        u32                          s_Serial             = 0;
        u32                          s_PointerEnterSerial = 0;
        WaylandWindow*               s_FocusedWindow      = nullptr;
        WaylandWindow*               s_KeyboardFocus      = nullptr;
    }; // namespace

    using Input::KeyCode;

    static KeyCode ToVtKeyCode(u32 scancode)
    {
        KeyCode keyCode = KeyCode::eUnknown;
        switch (scancode)
        {
            case KEY_0: keyCode = KeyCode::eNum0; break;
            case KEY_1: keyCode = KeyCode::eNum1; break;
            case KEY_2: keyCode = KeyCode::eNum2; break;
            case KEY_3: keyCode = KeyCode::eNum3; break;
            case KEY_4: keyCode = KeyCode::eNum4; break;
            case KEY_5: keyCode = KeyCode::eNum5; break;
            case KEY_6: keyCode = KeyCode::eNum6; break;
            case KEY_7: keyCode = KeyCode::eNum7; break;
            case KEY_8: keyCode = KeyCode::eNum8; break;
            case KEY_9: keyCode = KeyCode::eNum9; break;
            case KEY_A: keyCode = KeyCode::eA; break;
            case KEY_B: keyCode = KeyCode::eB; break;
            case KEY_C: keyCode = KeyCode::eC; break;
            case KEY_D: keyCode = KeyCode::eD; break;
            case KEY_E: keyCode = KeyCode::eE; break;
            case KEY_F: keyCode = KeyCode::eF; break;
            case KEY_G: keyCode = KeyCode::eG; break;
            case KEY_H: keyCode = KeyCode::eH; break;
            case KEY_I: keyCode = KeyCode::eI; break;
            case KEY_J: keyCode = KeyCode::eJ; break;
            case KEY_K: keyCode = KeyCode::eK; break;
            case KEY_L: keyCode = KeyCode::eL; break;
            case KEY_M: keyCode = KeyCode::eM; break;
            case KEY_N: keyCode = KeyCode::eN; break;
            case KEY_O: keyCode = KeyCode::eO; break;
            case KEY_P: keyCode = KeyCode::eP; break;
            case KEY_Q: keyCode = KeyCode::eQ; break;
            case KEY_R: keyCode = KeyCode::eR; break;
            case KEY_S: keyCode = KeyCode::eS; break;
            case KEY_T: keyCode = KeyCode::eT; break;
            case KEY_U: keyCode = KeyCode::eU; break;
            case KEY_V: keyCode = KeyCode::eV; break;
            case KEY_W: keyCode = KeyCode::eW; break;
            case KEY_X: keyCode = KeyCode::eX; break;
            case KEY_Y: keyCode = KeyCode::eY; break;
            case KEY_Z: keyCode = KeyCode::eZ; break;
            case KEY_GRAVE: keyCode = KeyCode::eTilde; break;
            case KEY_F1: keyCode = KeyCode::eF1; break;
            case KEY_F2: keyCode = KeyCode::eF2; break;
            case KEY_F3: keyCode = KeyCode::eF3; break;
            case KEY_F4: keyCode = KeyCode::eF4; break;
            case KEY_F5: keyCode = KeyCode::eF5; break;
            case KEY_F6: keyCode = KeyCode::eF6; break;
            case KEY_F7: keyCode = KeyCode::eF7; break;
            case KEY_F8: keyCode = KeyCode::eF8; break;
            case KEY_F9: keyCode = KeyCode::eF9; break;
            case KEY_F10: keyCode = KeyCode::eF10; break;
            case KEY_F11: keyCode = KeyCode::eF11; break;
            case KEY_F12: keyCode = KeyCode::eF12; break;
            case KEY_F13: keyCode = KeyCode::eF13; break;
            case KEY_F14: keyCode = KeyCode::eF14; break;
            case KEY_F15: keyCode = KeyCode::eF15; break;
            case KEY_ESC: keyCode = KeyCode::eEscape; break;
            case KEY_BACKSPACE: keyCode = KeyCode::eBackspace; break;
            case KEY_TAB: keyCode = KeyCode::eTab; break;
            case KEY_CAPSLOCK: keyCode = KeyCode::eCapsLock; break;
            case KEY_ENTER: keyCode = KeyCode::eReturn; break;
            case KEY_KPENTER: keyCode = KeyCode::eEnter; break;
            case KEY_LEFTSHIFT: keyCode = KeyCode::eLShift; break;
            case KEY_RIGHTSHIFT: keyCode = KeyCode::eRShift; break;
            case KEY_LEFTCTRL: keyCode = KeyCode::eLCtrl; break;
            case KEY_RIGHTCTRL: keyCode = KeyCode::eRCtrl; break;
            case KEY_LEFTALT: keyCode = KeyCode::eLAlt; break;
            case KEY_RIGHTALT: keyCode = KeyCode::eRAlt; break;
            case KEY_LEFTMETA: keyCode = KeyCode::eLSystem; break;
            case KEY_RIGHTMETA: keyCode = KeyCode::eRSystem; break;
            case KEY_SPACE: keyCode = KeyCode::eSpace; break;
            case KEY_MINUS: keyCode = KeyCode::eHyphen; break;
            case KEY_EQUAL: keyCode = KeyCode::eEqual; break;
            case KEY_KPDOT: keyCode = KeyCode::eDecimal; break;
            case KEY_LEFTBRACE: keyCode = KeyCode::eLBracket; break;
            case KEY_RIGHTBRACE: keyCode = KeyCode::eRBracket; break;
            case KEY_SEMICOLON: keyCode = KeyCode::eSemicolon; break;
            case KEY_APOSTROPHE: keyCode = KeyCode::eApostrophe; break;
            case KEY_COMMA: keyCode = KeyCode::eComma; break;
            case KEY_DOT: keyCode = KeyCode::ePeriod; break;
            case KEY_SLASH: keyCode = KeyCode::eSlash; break;
            case KEY_BACKSLASH: keyCode = KeyCode::eBackslash; break;
            case KEY_UP: keyCode = KeyCode::eUp; break;
            case KEY_DOWN: keyCode = KeyCode::eDown; break;
            case KEY_LEFT: keyCode = KeyCode::eLeft; break;
            case KEY_RIGHT: keyCode = KeyCode::eRight; break;
            case KEY_KP0: keyCode = KeyCode::eNumpad0; break;
            case KEY_KP1: keyCode = KeyCode::eNumpad1; break;
            case KEY_KP2: keyCode = KeyCode::eNumpad2; break;
            case KEY_KP3: keyCode = KeyCode::eNumpad3; break;
            case KEY_KP4: keyCode = KeyCode::eNumpad4; break;
            case KEY_KP5: keyCode = KeyCode::eNumpad5; break;
            case KEY_KP6: keyCode = KeyCode::eNumpad6; break;
            case KEY_KP7: keyCode = KeyCode::eNumpad7; break;
            case KEY_KP8: keyCode = KeyCode::eNumpad8; break;
            case KEY_KP9: keyCode = KeyCode::eNumpad9; break;
            case KEY_KPPLUS: keyCode = KeyCode::eAdd; break;
            case KEY_KPMINUS: keyCode = KeyCode::eSubtract; break;
            case KEY_KPASTERISK: keyCode = KeyCode::eMultiply; break;
            case KEY_KPSLASH: keyCode = KeyCode::eDivide; break;
            case KEY_INSERT: keyCode = KeyCode::eInsert; break;
            case KEY_DELETE: keyCode = KeyCode::eDelete; break;
            case KEY_PAGEUP: keyCode = KeyCode::ePageUp; break;
            case KEY_PAGEDOWN: keyCode = KeyCode::ePageDown; break;
            case KEY_HOME: keyCode = KeyCode::eHome; break;
            case KEY_END: keyCode = KeyCode::eEnd; break;
            case KEY_SCROLLLOCK: keyCode = KeyCode::eScrollLock; break;
            case KEY_NUMLOCK: keyCode = KeyCode::eNumLock; break;
            case KEY_PRINT: keyCode = KeyCode::ePrintScreen; break;
            case KEY_PAUSE: keyCode = KeyCode::ePause; break;
            case KEY_MENU: keyCode = KeyCode::eMenu; break;

            default: break;
        }

        return keyCode;
    }

    using Input::MouseCode;

    WaylandWindow::WaylandWindow(const WindowSpecification& specification)
        : Window(specification)
    {
        if (s_WindowsCount == 0)
        {
            WaylandWindow::Initialize();
            VtCoreInfo("Wayland: Successfully initialized");
        }

        [[maybe_unused]] i32         width  = specification.VideoMode.Width;
        [[maybe_unused]] i32         height = specification.VideoMode.Height;
        [[maybe_unused]] const char* title  = specification.Title.data();

        m_Data.Visible                      = specification.Visible;
        m_Data.Decorated                    = specification.Decorated;
        m_Data.Focused                      = specification.Focused;
        m_Data.Position                     = specification.Position;
        m_Data.AutoIconify                  = specification.AutoIconify;
        m_Data.FocusOnShow                  = specification.FocusOnShow;

        static constexpr xdg_surface_listener wm_surface_listener{
            .configure = [](void*, xdg_surface* surface, u32 serial) noexcept
            { xdg_surface_ack_configure(surface, serial); }};

        static constexpr xdg_toplevel_listener toplevel_listener{
            .configure =
                [](void* data, xdg_toplevel*, i32 width, i32 height,
                   wl_array*) noexcept
            {
                auto self = reinterpret_cast<WaylandWindow*>(data);

                self->m_Data.VideoMode.Width  = width;
                self->m_Data.VideoMode.Height = height;
                WindowEvents::WindowResizedEvent(self, width, height);
                self->m_RendererContext->OnResize();
            },
            .close =
                [](void* data, xdg_toplevel*) noexcept
            {
                auto self           = reinterpret_cast<WaylandWindow*>(data);

                self->m_Data.IsOpen = false;
            },
            .configure_bounds =
                [](void*, xdg_toplevel*, i32, i32) noexcept {

                },
            .wm_capabilities =
                [](void*, xdg_toplevel*, wl_array*) noexcept {

                }};

        auto surface = wl_compositor_create_surface(Wayland::GetCompositor());
        VtCoreSlowAssert(surface);

        wl_surface_set_user_data(surface, this);
        m_WmSurface
            = xdg_wm_base_get_xdg_surface(Wayland::GetWmBase(), surface);
        xdg_surface_add_listener(m_WmSurface, &wm_surface_listener, this);
        m_TopLevel = xdg_surface_get_toplevel(m_WmSurface);
        xdg_toplevel_add_listener(m_TopLevel, &toplevel_listener, this);
        wl_surface_commit(surface);
        m_WindowHandle = surface;

        if (Wayland::GetAlphaModifier())
            m_AlphaModifierSurface = wp_alpha_modifier_v1_get_surface(
                Wayland::GetAlphaModifier(), m_WindowHandle);

        SetOpacity(1.0f);

        GetWindowMap()[m_WindowHandle] = this;

        VtCoreTrace(
            "Wayland: Created window {{ width: {}, height: {}, title: {} }}",
            width, height, title);

        ++s_WindowsCount;
        SetVisible(true);
        m_Data.IsOpen = true;

        SetupEvents();

        if (!specification.NoAPI)
            m_RendererContext
                = RendererContext::Create(this, specification.VSync);
    }
    WaylandWindow::~WaylandWindow()
    {
        m_RendererContext.reset();
        VtCoreTrace("Wayland: Destroying window...");
        wp_alpha_modifier_surface_v1_destroy(m_AlphaModifierSurface);
        xdg_toplevel_destroy(m_TopLevel);
        xdg_surface_destroy(m_WmSurface);
        wl_surface_destroy(m_WindowHandle);
        --s_WindowsCount;
        if (s_WindowsCount == 0) Shutdown();
    }

    void WaylandWindow::PollEvents()
    {
        auto pollSingle = [](i32 fd, i16 events, i32 timeout) -> u16
        {
            pollfd pfd{.fd = fd, .events = events, .revents = 0};
            VtCoreAssert(poll(&pfd, 1, timeout) >= 0);

            return pfd.revents;
        };

        wl_display* display = Wayland::GetDisplay();
        while (wl_display_prepare_read(display))
            wl_display_dispatch_pending(display);

        while (wl_display_flush(display) < 0 && EAGAIN == errno)
            pollSingle(wl_display_get_fd(display), POLLOUT, -1);

        if (POLLIN & pollSingle(wl_display_get_fd(display), POLLIN, 0))
        {
            wl_display_read_events(display);
            wl_display_dispatch_pending(display);
        }
        else wl_display_cancel_read(display);

        VtCoreAssert(wl_display_get_error(display) == 0);
    }
    void WaylandWindow::Present()
    {
        if (m_RendererContext) m_RendererContext->Present();
    }

    bool WaylandWindow::IsFocused() const noexcept { return m_Data.Focused; }
    bool WaylandWindow::IsMinimized() const noexcept
    {
        VT_TODO();
        return false;
    }
    bool WaylandWindow::IsHovered() const noexcept
    {
        return s_FocusedWindow == this;
    }
    std::string WaylandWindow::GetTitle() const noexcept
    {
        return m_Data.Title;
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
        return GetSize();
    }
    Vec2f WaylandWindow::GetContentScale() const noexcept
    {
        return {0.0f, 0.0f};
    }
    f32  WaylandWindow::GetOpacity() const noexcept { return 1.0f; }

    void WaylandWindow::Close() noexcept { m_Data.IsOpen = false; }
    void WaylandWindow::RequestFocus() noexcept { VT_TODO(); }
    void WaylandWindow::RequestUserAttention() const noexcept { VT_TODO(); }
    void WaylandWindow::Maximize() noexcept { VT_TODO(); }
    void WaylandWindow::Minimize() noexcept { VT_TODO(); }
    void WaylandWindow::Restore() noexcept { VT_TODO(); }
    void WaylandWindow::SetTitle(std::string_view title)
    {
        m_Data.Title = title;
        VT_TODO();
    }
    void WaylandWindow::SetIcon(const Icon* icons, usize count)
    {
        (void)icons;
        (void)count;
        VtCoreWarn(
            "Wayland: The platform doesn't support setting the window icon");
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
        VT_TODO();
    }
    void WaylandWindow::SetSize(const Vec2i& size) noexcept
    {
        (void)size;
        VT_TODO();
    }
    void WaylandWindow::SetOpacity(f32 opacity)
    {
        static constexpr f64 maxUint32 = std::numeric_limits<u32>::max();
        u32                  alpha     = maxUint32 * opacity;

        if (m_AlphaModifierSurface)
            return wp_alpha_modifier_surface_v1_set_multiplier(
                m_AlphaModifierSurface, alpha);

        VtCoreWarn(
            "Wayland: The compositor does not support setting the window's "
            "opacity");
    }
    void WaylandWindow::SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                     i32 maxHeight)
    {
        m_Data.MinWidth  = minWidth;
        m_Data.MinHeight = minHeight;
        m_Data.MaxWidth  = maxWidth;
        m_Data.MaxHeight = maxHeight;
        VT_TODO();
    }

    void WaylandWindow::SetAutoIconify(bool autoIconify) const noexcept
    {
        VT_TODO();
        (void)autoIconify;
    }
    void WaylandWindow::SetCursorPosition(Vec2d position) noexcept
    {
        VT_UNUSED(position);
        VtCoreWarn(
            "Wayland: The platform does not support setting cursor position");
    }
    void WaylandWindow::ShowCursor() const noexcept { VT_TODO(); }
    void WaylandWindow::HideCursor() const noexcept { VT_TODO(); }
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
        VT_TODO();
    }
    void WaylandWindow::SetVisible(bool visible) const
    {
        VT_TODO();
        (void)visible;
    }
    void WaylandWindow::SetAlwaysOnTop(bool alwaysOnTop)
    {
        VT_UNUSED(alwaysOnTop);
        VtCoreWarn(
            "Wayland: The platform does not support making a window floating");
    }

    void WaylandWindow::SetupEvents() { using namespace WindowEvents; }

    void WaylandWindow::Initialize()
    {
        Wayland::Initialize();

        static wl_pointer_listener pointerListener{};
        pointerListener.enter  = PointerHandleEnter;
        pointerListener.leave  = PointerHandleLeave;
        pointerListener.button = PointerHandleButton;
        pointerListener.motion = PointerHandleMotion;
        pointerListener.axis   = PointerHandleAxis;
        Wayland::SetPointerListener(&pointerListener);

        static wl_keyboard_listener keyboardListener{};
        keyboardListener.enter = KeyboardHandleEnter;
        keyboardListener.leave = KeyboardHandleLeave;
        keyboardListener.key   = KeyboardHandleKey;
        Wayland::SetKeyboardListener(&keyboardListener);
    }
    void WaylandWindow::Shutdown()
    {
        VtCoreTrace("Wayland: Shutting down");
        Wayland::Shutdown();
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

        WaylandWindow* window  = WaylandWindow::GetWindowMap()[surface];
        s_Serial               = serial;
        window->m_Data.Focused = false;
        s_FocusedWindow        = nullptr;
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

        WaylandWindow* window    = s_FocusedWindow;
        MouseCode      mouseCode = static_cast<MouseCode>(button - BTN_LEFT);
        if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            WindowEvents::MouseButtonPressedEvent(window, mouseCode);
        else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            WindowEvents::MouseButtonReleasedEvent(window, mouseCode);
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

        WaylandWindow* window = WaylandWindow::GetWindowMap()[surface];
        s_KeyboardFocus       = window;
    }
    void WaylandWindow::KeyboardHandleLeave(void*        userData,
                                            wl_keyboard* keyboard, u32 serial,
                                            wl_surface* surface)
    {
        VT_UNUSED(userData);
        VT_UNUSED(keyboard);
        VT_UNUSED(serial);
        VT_UNUSED(surface);
        s_KeyboardFocus = nullptr;
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

        KeyCode key = ToVtKeyCode(scancode);
        if (state == WL_KEYBOARD_KEY_STATE_PRESSED && key == KeyCode::eEscape
            && s_KeyboardFocus)
            s_KeyboardFocus->m_Data.IsOpen = false;

        if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
            WindowEvents::KeyPressedEvent(s_KeyboardFocus, key, 0);
        else if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
            WindowEvents::KeyReleasedEvent(s_KeyboardFocus, key);
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
