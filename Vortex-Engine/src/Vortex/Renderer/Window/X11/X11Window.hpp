/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/Window/Window.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    class VT_API X11Window : public Window
    {
      public:
        X11Window(const WindowSpecification& specification);
        virtual ~X11Window();

        static void             PollEvents();
        virtual void            Present() override;

        inline virtual std::any GetNativeHandle() const noexcept override
        {
            return m_Window;
        }
        inline virtual bool IsOpen() const noexcept override
        {
            return m_Data.IsOpen;
        }
        virtual bool        IsFocused() const noexcept override;
        virtual bool        IsMinimized() const noexcept override;
        virtual bool        IsHovered() const noexcept override;
        virtual std::string GetTitle() const noexcept override;
        virtual Vec2i       GetPosition() const noexcept override;
        virtual Vec2i       GetSize() const noexcept override;
        virtual Vec2i       GetFramebufferSize() const noexcept override;
        virtual Vec2f       GetContentScale() const noexcept override;
        virtual f32         GetOpacity() const noexcept override;

        virtual void        Close() noexcept override;
        virtual void        RequestFocus() noexcept override;
        virtual void        RequestUserAttention() const noexcept override;
        virtual void        Maximize() noexcept override;
        virtual void        Minimize() noexcept override;
        virtual void        Restore() noexcept override;

        virtual void        SetTitle(std::string_view title) override;
        virtual void        SetIcon(const Icon* icons, usize count) override;
        virtual void        SetPosition(i32 x, i32 y) const override;
        virtual void SetAspectRatio(i32 numerator, i32 denominator) override;
        virtual void SetWidth(const i32 width) noexcept override;
        virtual void SetHeight(const i32 height) noexcept override;

        virtual void SetOpacity(f32 opacity) override;

        virtual void SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                  i32 maxHeight) override;

        virtual void SetAutoIconify(bool autoIconify) const noexcept override;
        virtual void SetCursorPosition(Vec2d position) noexcept override;
        virtual void ShowCursor() const noexcept override;
        virtual void HideCursor() const noexcept override;
        virtual void SetFullscreen(bool fullscreen) override;
        virtual void SetResizable(bool resizable) noexcept override;
        virtual void SetVisible(bool visible) const override;
        virtual void SetAlwaysOnTop(bool alwaysOnTop) override;

      private:
        static usize s_WindowsCount;
        GLFWwindow*  m_Window;

        void         SetupEvents();

        static bool  Initialize();
        static void  Shutdown();
    };
}; // namespace Vortex
