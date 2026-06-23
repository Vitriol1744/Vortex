/*
 * Created by v1tr10l7 on 20.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/API/SwapChain.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex
{
    class RenderTargetWindow : public NonCopyable<RenderTargetWindow>
    {
      public:
        inline RenderTargetWindow(const WindowSpecification& specs)
        {
            m_Window = Window::Create(specs);
            VtCoreAssertMsg(
                m_Window,
                "Failed to create native platform window interface context.");

            if (!specs.NoAPI)
                m_SwapChain = SwapChain::Create(m_Window.Raw(), specs.VSync);
            VtCoreAssertMsg(m_SwapChain,
                            "Native window implementation failed to create an "
                            "internal graphics SwapChain boundary context.");

            Window::SubscribeEvent<WindowResizedEvent>(
                [this](const WindowResizedEvent& e)
                { return OnWindowResized(e); });
        }

        ~RenderTargetWindow() = default;

        inline void Present()
        {
            if (m_SwapChain) m_SwapChain->Present();
        }

        inline void       Close() noexcept { m_Window->Close(); }

        inline ::std::any NativeHandle() const noexcept
        {
            return m_Window->NativeHandle();
        }

        inline class Window&        Window() noexcept { return *m_Window; }
        inline Ref<class SwapChain> SwapChain() const noexcept
        {
            return m_SwapChain;
        }

        inline bool IsOpen() const noexcept
        {
            return m_Window && m_Window->IsOpen();
        }
        inline bool IsFocused() const noexcept { return m_Window->IsFocused(); }
        inline bool IsMinimized() const noexcept
        {
            return m_Window->IsMinimized();
        }
        inline bool IsHovered() const noexcept { return m_Window->IsHovered(); }

        inline String      Title() const noexcept { return m_Window->Title(); }
        inline Math::Vec2i Position() const noexcept
        {
            return m_Window->Position();
        }
        inline Math::Vec2i Size() const noexcept { return m_Window->Size(); }
        inline i32         Width() const noexcept { return m_Window->Width(); }
        inline i32 Height() const noexcept { return m_Window->Height(); }
        inline Math::Vec2i FramebufferSize() const noexcept
        {
            return m_Window->FramebufferSize();
        }
        inline Math::Vec2f ContentScale() const noexcept
        {
            return m_Window->ContentScale();
        }
        inline f32 Opacity() const noexcept { return m_Window->Opacity(); }
        inline Math::Vec2d CursorPosition() const noexcept
        {
            return m_Window->CursorPosition();
        }

        inline void RequestFocus() noexcept { m_Window->RequestFocus(); }
        inline void RequestUserAttention() const noexcept
        {
            m_Window->RequestUserAttention();
        }
        inline void Maximize() noexcept { m_Window->Maximize(); }
        inline void Minimize() noexcept { m_Window->Minimize(); }
        inline void Restore() noexcept { m_Window->Restore(); }

        // Property Alteration Setters
        inline void SetTitle(StringView title) { m_Window->SetTitle(title); }
        inline void SetIcon(const Icon& icon) { m_Window->SetIcon(icon); }
        inline void SetIcon(const Icon* icons, usize count)
        {
            m_Window->SetIcon(icons, count);
        }
        inline void SetPosition(const Math::Vec2i& position) noexcept
        {
            m_Window->SetPosition(position);
        }
        inline void SetPosition(i32 x, i32 y) { m_Window->SetPosition(x, y); }
        inline void SetAspectRatio(i32 numerator, i32 denominator)
        {
            m_Window->SetAspectRatio(numerator, denominator);
        }
        inline void SetSize(const Math::Vec2i& size) noexcept
        {
            m_Window->SetSize(size);
        }
        inline void SetWidth(i32 width) noexcept { m_Window->SetWidth(width); }
        inline void SetHeight(i32 height) noexcept
        {
            m_Window->SetHeight(height);
        }
        inline void SetOpacity(f32 opacity) { m_Window->SetOpacity(opacity); }
        inline void SetSizeLimit(i32 minWidth, i32 minHeight, i32 maxWidth,
                                 i32 maxHeight)
        {
            m_Window->SetSizeLimit(minWidth, minHeight, maxWidth, maxHeight);
        }

        inline void SetAutoIconify(bool autoIconify) const noexcept
        {
            m_Window->SetAutoIconify(autoIconify);
        }
        inline void SetCursorPosition(Math::Vec2d position) noexcept
        {
            m_Window->SetCursorPosition(position);
        }
        inline void ShowCursor() const noexcept { m_Window->ShowCursor(); }
        inline void HideCursor() const noexcept { m_Window->HideCursor(); }
        inline void SetFullscreen(bool fullscreen)
        {
            m_Window->SetFullscreen(fullscreen);
        }
        inline void SetResizable(bool resizable) noexcept
        {
            m_Window->SetResizable(resizable);
        }
        inline void SetVisible(bool visible) const
        {
            m_Window->SetVisible(visible);
        }
        inline void SetAlwaysOnTop(bool alwaysOnTop)
        {
            m_Window->SetAlwaysOnTop(alwaysOnTop);
        }

      private:
        Scope<class Window>  m_Window;
        Ref<class SwapChain> m_SwapChain;

        void                 OnWindowResized(const WindowResizedEvent& e)
        {
            if (m_Window.Raw() != e.Window) return;
            m_SwapChain->OnResize();
        }
    };
} // namespace Vortex
