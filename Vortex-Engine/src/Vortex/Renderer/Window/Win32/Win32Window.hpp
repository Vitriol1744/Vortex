/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Win32/Windows.hpp"
#include "Vortex/Renderer/Window/Window.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    class VT_API Win32Window : public Window
    {
      public:
        Win32Window(const WindowSpecification& specification);
        virtual ~Win32Window();

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

        virtual void        Close() noexcept override;
        virtual void        RequestFocus() noexcept override;
        virtual void        RequestUserAttention() const noexcept override;
        virtual void        Maximize() noexcept override;
        virtual void        Minimize() noexcept override;
        virtual void        Restore() noexcept override;

        virtual void        SetTitle(std::string_view title) override;
        virtual void        SetIcon(const Icon* icons, usize count) override;
        virtual void        SetPosition(i32 x, i32 y) override;
        virtual void SetAspectRatio(i32 numerator, i32 denominator) override;
        virtual void SetSize(const Vec2i& size) noexcept override;

        virtual void SetOpacity(f32 opacity) override;
        virtual Ref<RendererContext>
        GetRendererContext() const noexcept override
        {
            return m_RendererContext;
        }

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
        HWND                       m_WindowHandle  = nullptr;
        HICON                      m_BigIcon       = nullptr;
        HICON                      m_SmallIcon     = nullptr;
        bool                       m_CursorTracked = false;

        GLFWwindow*                m_Window;
        Ref<class RendererContext> m_RendererContext = nullptr;

        Vec2i                      m_LastCursorPos   = {0, 0};

        static usize               s_WindowsCount;
        static std::unordered_map<HWND, Win32Window*>& GetWindowMap()
        {
            static std::unordered_map<HWND, Win32Window*> map;

            return map;
        }
        inline static std::unordered_map<HWND, Win32Window*>* GetWindowsMap()
        {
            static auto windowsMap
                = new std::unordered_map<HWND, Win32Window*>();

            return windowsMap;
        }

        DWORD                 GetWindowStyle();
        DWORD                 GetWindowStyleEx();
        void                  SetupEvents();
        void                  ApplyAspectRatio(i32 edge, RECT& area);
        LRESULT WINAPI        HandleEvents(HWND hWnd, UINT msg, WPARAM wParam,
                                           LPARAM lParam);

        static LRESULT WINAPI HandleGlobalEvents(HWND hWnd, UINT msg,
                                                 WPARAM wParam, LPARAM lParam);
        static Image& ChooseImage(const Image* images, usize count, i32 width,
                                  i32 height);
        static HICON  CreateIconOrCursor(const Image& image, i32 xhot, i32 yhot,
                                         bool icon = true);

        static bool   Initialize();
        static void   Shutdown();
    };
}; // namespace Vortex
