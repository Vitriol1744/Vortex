/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/Window/Wayland/Wayland.hpp"
#include "Vortex/Renderer/Window/Window.hpp"

#include <wayland-xdg-shell-client-protocol.h>

namespace Vortex
{
    class WaylandWindow : public Window
    {
      public:
        WaylandWindow(const WindowSpecification& specification);
        virtual ~WaylandWindow();

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

        static auto& GetWindowMap()
        {
            static std::unordered_map<wl_surface*, WaylandWindow*> windowMap
                = {};

            return windowMap;
        }

      public:
        wl_surface*                   m_WindowHandle         = nullptr;
        xdg_surface*                  m_WmSurface            = nullptr;
        xdg_toplevel*                 m_TopLevel             = nullptr;
        wp_alpha_modifier_surface_v1* m_AlphaModifierSurface = nullptr;
        Ref<class RendererContext>    m_RendererContext      = nullptr;

        static usize                  s_WindowsCount;

        void                          SetupEvents();

        static bool                   Initialize();
        static void                   Shutdown();

        static void PointerHandleEnter(void* userData, wl_pointer* pointer,
                                       u32 serial, wl_surface* surface,
                                       wl_fixed_t xOffset, wl_fixed_t yOffset);
        static void PointerHandleLeave(void* userData, wl_pointer* pointer,
                                       u32 serial, wl_surface* surface);
        static void PointerHandleMotion(void* userData, wl_pointer* pointer,
                                        u32 time, wl_fixed_t xOffset,
                                        wl_fixed_t yOffset);
        static void PointerHandleButton(void* userData, wl_pointer* pointer,
                                        u32 serial, u32 time, u32 button,
                                        u32 state);
        static void PointerHandleAxis(void* userData, wl_pointer* pointer,
                                      u32 time, u32 axis, wl_fixed_t value);
        static void PointerHandleFrame(void* userData, wl_pointer* pointer);
        static void PointerHandleAxisSource(void* userData, wl_pointer* pointer,
                                            u32 axisSource);
        static void PointerHandleAxisStop(void* userData, wl_pointer* pointer,
                                          u32 time, u32 axis);
        static void PointerHandleAxisDiscrete(void*       userData,
                                              wl_pointer* pointer, u32 axis,
                                              i32 discrete);
        static void PointerHandleAxisValue120(void*       userData,
                                              wl_pointer* pointer, u32 axis,
                                              i32 value120);
        static void PointerHandleAxisRelativeDirection(void*       userData,
                                                       wl_pointer* pointer,
                                                       u32 axis, u32 direction);

        static void KeyboardHandleKeymap(void* userData, wl_keyboard* keyboard,
                                         u32 format, int fd, u32 size);
        static void KeyboardHandleEnter(void* userData, wl_keyboard* keyboard,
                                        u32 serial, wl_surface* surface,
                                        wl_array* keys);
        static void KeyboardHandleLeave(void* userData, wl_keyboard* keyboard,
                                        u32 serial, wl_surface* surface);
        static void KeyboardHandleKey(void* userData, wl_keyboard* keyboard,
                                      u32 serial, u32 time, u32 scancode,
                                      u32 state);
        static void KeyboardHandleModifiers(void*        userData,
                                            wl_keyboard* keyboard, u32 serial,
                                            u32 modsDepressed, u32 modsLatched,
                                            u32 modsLocked, u32 group);
        static void KeyboardHandleRepeatInfo(void*        userData,
                                             wl_keyboard* keyboard, i32 rate,
                                             i32 delay);
    };
}; // namespace Vortex
