/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Window/Window.hpp"

#include "Vortex/Window/X11/X11.hpp"

#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>

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
            return m_WindowHandle;
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
        virtual Vec2d       GetCursorPosition() const noexcept override;

        virtual Ref<RendererContext>
        GetRendererContext() const noexcept override
        {
            return m_RendererContext;
        }

        virtual void Close() noexcept override;
        virtual void RequestFocus() noexcept override;
        virtual void RequestUserAttention() const noexcept override;
        virtual void Maximize() noexcept override;
        virtual void Minimize() noexcept override;
        virtual void Restore() noexcept override;

        virtual void SetTitle(std::string_view title) override;
        virtual void SetIcon(const Icon* icons, usize count) override;
        virtual void SetPosition(i32 x, i32 y) override;
        virtual void SetAspectRatio(i32 numerator, i32 denominator) override;
        virtual void SetSize(const Vec2i& size) noexcept override;

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
        xcb_window_t               m_WindowHandle;
        XIC                        m_InputContext = nullptr;
        xcb_window_t               m_Parent;
        bool                       m_MouseHovered    = false;
        Ref<class RendererContext> m_RendererContext = nullptr;
        Vec2i                      m_LastCursorPos   = {0, 0};
        Vec2i                      m_WarpCursorPos   = {0, 0};

        static usize               s_WindowsCount;

        static auto&               GetWindowMap()
        {
            static std::unordered_map<::Window, X11Window*> windowMap = {};

            return windowMap;
        }
        static void ProcessEvent(XEvent& event);
    };
}; // namespace Vortex
