/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Events/Event.hpp"
#include "Vortex/Core/Input/KeyCode.hpp"
#include "Vortex/Core/Input/MouseCode.hpp"
#include "Vortex/Core/Math/Vector.hpp"
#include "Vortex/Core/NonCopyable.hpp"
#include "Vortex/Core/Types.hpp"

#include "Vortex/Renderer/API/RendererContext.hpp"
#include "Vortex/Renderer/Window/Monitor.hpp"

namespace Vortex
{
    enum class WindowSubsystem
    {
        eUndefined,
        eWin32,
        eX11,
        eWayland,
        eCocoa,
    };

    using Pixel = u8;
    struct Icon
    {
        i32    Width  = 0;
        i32    Height = 0;
        Pixel* Pixels;
    };

    struct WindowSpecification
    {
        WindowSpecification() = default;
        WindowSpecification(const VideoMode& videoMode, std::string_view title)
            : VideoMode(videoMode)
            , Title(title)
        {
        }

        struct VideoMode   VideoMode;
        std::string        Title        = "Vortex Application";
        Vec2i              Position     = {0, 0};
        Ref<class Monitor> Monitor      = nullptr;

        bool               Fullscreen   = false;
        bool               Resizable    = true;
        bool               Visible      = true;
        bool               Decorated    = false;
        bool               Focused      = true;
        bool               AlwaysOnTop  = true;
        bool               Maximized    = false;
        bool               AutoIconify  = false;
        bool               CenterCursor = false;
        bool               FocusOnShow  = false;
    };

    class VT_API Window : public NonCopyable<Window>
    {
      public:
        Window()          = default;
        virtual ~Window() = default;

        static void         PollEvents();
        virtual void        Present()                        = 0;

        virtual std::any    GetNativeHandle() const noexcept = 0;
        virtual bool        IsOpen() const noexcept          = 0;
        virtual bool        IsFocused() const noexcept       = 0;
        virtual bool        IsMinimized() const noexcept     = 0;
        virtual bool        IsHovered() const noexcept       = 0;
        virtual std::string GetTitle() const noexcept        = 0;
        virtual Vec2i       GetPosition() const noexcept     = 0;
        inline Vec2i        GetSize() const noexcept
        {
            return Vec2i(GetWidth(), GetHeight());
        }
        virtual i32   GetWidth() const noexcept           = 0;
        virtual i32   GetHeight() const noexcept          = 0;
        virtual Vec2i GetFramebufferSize() const noexcept = 0;
        virtual Vec2f GetContentScale() const noexcept    = 0;
        virtual f32   GetOpacity() const noexcept         = 0;
        inline Vec2d  GetCursorPosition() const noexcept
        {
            return m_Data.MousePosition;
        }

        inline Ref<RendererContext> GetRendererContext() const noexcept
        {
            return m_Data.RendererContext;
        }

        virtual void Close() noexcept                      = 0;
        virtual void RequestFocus() noexcept               = 0;
        virtual void RequestUserAttention() const noexcept = 0;
        virtual void Maximize() noexcept                   = 0;
        virtual void Minimize() noexcept                   = 0;
        virtual void Restore() noexcept                    = 0;

        virtual void SetTitle(std::string_view title)      = 0;
        virtual void SetIcon(i32 count, const std::vector<Icon>& icons) const
            = 0;

        inline void SetPosition(const Vec2i& position) noexcept
        {
            SetPosition(position.x, position.y);
        }
        virtual void SetPosition(i32 x, i32 y) const                = 0;
        virtual void SetAspectRatio(i32 numerator, i32 denominator) = 0;
        inline void  SetSize(const Vec2i& size) noexcept
        {
            SetWidth(size.x);
            SetHeight(size.y);
        }
        virtual void SetWidth(const i32 width) noexcept   = 0;
        virtual void SetHeight(const i32 height) noexcept = 0;
        virtual void SetOpacity(f32 opacity)              = 0;

        virtual void SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                  i32 maxHeight)
            = 0;

        virtual void SetAutoIconify(bool autoIconify) const noexcept  = 0;
        virtual void SetCursorPosition(Vec2d position) const noexcept = 0;
        virtual void ShowCursor() const noexcept                      = 0;
        virtual void HideCursor() const noexcept                      = 0;
        virtual void SetFullscreen(bool fullscreen)                   = 0;
        virtual void SetResizable(bool resizable) noexcept            = 0;
        virtual void SetVisible(bool visible) const                   = 0;
        virtual void SetAlwaysOnTop(bool alwaysOnTop)                 = 0;

        static Ref<Window>
        Create(const WindowSpecification& windowSpecification);
        static Ref<Window> Create(const VideoMode& videoMode,
                                  std::string_view title)
        {
            return Create(WindowSpecification(videoMode, title));
        }
        static Ref<Window> Create(u32 width, u32 height, std::string_view title)
        {
            return Create(VideoMode(width, height), title);
        }

        static WindowSubsystem GetWindowSubsystem();

      protected:
        struct WindowData
        {
            struct VideoMode VideoMode;
            std::string      Title;
            Vec2i            Position         = {0, 0};
            bool             IsOpen           = true;
            bool             Fullscreen       = false;
            bool             Resizable        = true;
            bool             Visible          = true;
            bool             Focused          = false;
            bool             MouseHovered     = false;
            bool             Decorated        = false;
            bool             FocusOnShow      = false;
            bool             AutoIconify      = false;
            bool             MousePassthrough = false;
            i32              MinWidth         = 0;
            i32              MinHeight        = 0;
            i32              MaxWidth         = 0;
            i32              MaxHeight        = 0;
            i32              Numererator      = 0;
            i32              Denominator      = 0;

            std::array<bool, std::to_underlying(Input::KeyCode::eKeyCount)>
                Keys;
            std::array<bool, std::to_underlying(Input::MouseCode::eButtonCount)>
                                       MouseButtons;

            Vec2d                      MousePosition   = {0, 0};

            Ref<class RendererContext> RendererContext = nullptr;
        } m_Data;
    };

    namespace WindowEvents
    {
        // Window Events

        // Vortex::input::KeyCode keycode, u32 repeatCount
        extern Event<Window*, Input::KeyCode, u32> KeyPressedEvent;
        // Vortex::Input::KeyCode keycode
        extern Event<Window*, Input::KeyCode>      KeyReleasedEvent;
        // u32 codepoint
        extern Event<Window*, u32>                 KeyTypedEvent;
        // u32 codepoint, KeyCode modifier key
        extern Event<Window*, u32, Input::KeyCode> UnicodeKeyTypedEvent;
        // Vortex::Input::MouseCode mouseCode
        extern Event<Window*, Input::MouseCode>    MouseButtonPressedEvent;
        // Vortex::Input::MouseCode mouseCode
        extern Event<Window*, Input::MouseCode>    MouseButtonReleasedEvent;
        //
        extern Event<Window*>                      MouseEnteredEvent;
        //
        extern Event<Window*>                      MouseLeavedEvent;
        // Vortex::Vec2 wheel
        extern Event<Window*, f64, f64>            MouseScrolledEvent;
        // Vortex::Vec2 mousePosition
        extern Event<Window*, f64, f64>            MouseMovedEvent;
        //
        extern Event<Window*, i32, i32>            WindowMovedEvent;
        // Vortex::Window::WindowResizedEventType windowResizedEventType,
        // Vortex::Vec2u windowSize;
        extern Event<Window*, i32, i32>            WindowResizedEvent;
        //
        extern Event<Window*>                      WindowFocusedEvent;
        //
        extern Event<Window*>                      WindowFocusLostEvent;
        // bool maximized - if true window was maximized, else it was restored
        extern Event<Window*, bool>                WindowMaximizedEvent;
        // bool minimized - if true window was iconified, else it was restored
        extern Event<Window*, bool>                WindowMinimizedEvent;
        //
        extern Event<Window*>                      WindowClosedEvent;
        extern Event<Window*, i32, i32>            FramebufferResizedEvent;
        //
        extern Event<Window*, std::string>         FileDroppedEvent;
        // TODO(v1tr10l7) should it be moved somewhere else?
        extern Event<i32>                          GamepadConnectedEvent;
        //
        extern Event<i32>                          GamepadDisconnectedEvent;
    } // namespace WindowEvents

}; // namespace Vortex