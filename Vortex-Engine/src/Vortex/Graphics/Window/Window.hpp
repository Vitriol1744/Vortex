/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Math/Vector.hpp"
#include "Vortex/Core/NonCopyable.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    enum class MonitorState
    {
        eOn,
        eOff,
        eLowPower,
        eScreenSaver
    };

    struct VideoMode
    {
        VideoMode() = default;
        VideoMode(i32 width, i32 height, i32 bitsPerPixel = 32)
            : Width(width)
            , Height(height)
            , BitsPerPixel(bitsPerPixel)
        {
        }

        i32  Width                                     = 800;
        i32  Height                                    = 600;
        i32  BitsPerPixel                              = 32;

        auto operator<=>(const VideoMode& other) const = default;
    };

    struct WindowSpecification
    {
        WindowSpecification() = default;
        WindowSpecification(const VideoMode& videoMode, std::string_view title)
            : VideoMode(videoMode)
            , Title(title)
        {
        }

        struct VideoMode VideoMode;
        std::string      Title       = "Vortex Application";
        Vec2i            Position    = {0, 0};

        bool             Fullscreen  = false;
        bool             Resizable   = true;
        bool             Visible     = true;
        bool             Decorated   = false;
        bool             Focused     = true;
        bool             AlwaysOnTop = true;
        bool             Maximized   = false;
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
        virtual i32  GetWidth() const noexcept        = 0;
        virtual i32  GetHeight() const noexcept       = 0;

        virtual void Close() noexcept                 = 0;
        virtual void RequestFocus() noexcept          = 0;
        virtual void Maximize() noexcept              = 0;
        virtual void Minimize() noexcept              = 0;
        virtual void SetTitle(std::string_view title) = 0;
        inline void  SetPosition(const Vec2i& position) noexcept
        {
            SetPosition(position.x, position.y);
        }
        virtual void SetPosition(i32 x, i32 y) const = 0;
        inline void  SetSize(const Vec2i& size) noexcept
        {
            SetWidth(size.x);
            SetHeight(size.y);
        }
        virtual void SetWidth(const i32 width) noexcept    = 0;
        virtual void SetHeight(const i32 height) noexcept  = 0;
        virtual void ShowCursor() const noexcept           = 0;
        virtual void HideCursor() const noexcept           = 0;
        virtual void SetFullscreen(bool fullscreen)        = 0;
        virtual void SetResizable(bool resizable) noexcept = 0;
        virtual void SetVisible(bool visible) const        = 0;
        virtual void SetAlwaysOnTop(bool alwaysOnTop)      = 0;

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

      protected:
        struct WindowData
        {
            Window*          Window;
            struct VideoMode VideoMode;
            std::string      Title;
            Vec2i            Position     = {0, 0};
            bool             IsOpen       = true;
            bool             Fullscreen   = false;
            bool             Resizable    = true;
            bool             Visible      = true;
            bool             Focused      = false;
            bool             MouseHovered = false;
            bool             Decorated    = false;
        } m_Data;
    };
}; // namespace Vortex
