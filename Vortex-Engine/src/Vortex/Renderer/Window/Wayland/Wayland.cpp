/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Window/Wayland/Wayland.hpp"

namespace Vortex::Wayland
{
    namespace
    {
        wl_display*                    s_Display       = nullptr;
        wl_registry*                   s_Registry      = nullptr;
        wl_compositor*                 s_Compositor    = nullptr;
        wl_subcompositor*              s_Subcompositor = nullptr;
        wl_shm*                        s_Shm           = nullptr;

        std::forward_list<u32>         s_Outputs{};
        wl_seat*                       s_Seat                = nullptr;
        wl_pointer*                    s_Pointer             = nullptr;
        wl_keyboard*                   s_Keyboard            = nullptr;

        xdg_wm_base*                   s_WmBase              = nullptr;
        wp_alpha_modifier_v1*          s_AlphaModifier       = nullptr;
        zwlr_gamma_control_manager_v1* s_GammaControlManager = nullptr;

        wl_pointer_listener*           s_PointerListener     = nullptr;
        wl_keyboard_listener*          s_KeyboardListener    = nullptr;

        void SeatHandleCapabilities(void* userData, wl_seat* seat,
                                    u32 capabilities);
        void SeatHandleName(void* userData, wl_seat* seat, const char* name);

        void RegistryHandleGlobal(void* userData, wl_registry* registry,
                                  u32 name, const char* interfaceName,
                                  u32 version)
        {
            VT_UNUSED(userData);

            std::string_view interface = interfaceName;

            if (interface == wl_compositor_interface.name)
                s_Compositor
                    = reinterpret_cast<wl_compositor*>(wl_registry_bind(
                        registry, name, &wl_compositor_interface, 1));
            else if (interface == wl_subcompositor_interface.name)
                s_Subcompositor
                    = reinterpret_cast<wl_subcompositor*>(wl_registry_bind(
                        registry, name, &wl_subcompositor_interface, 1));
            else if (interface == wl_shm_interface.name)
                s_Shm = reinterpret_cast<wl_shm*>(
                    wl_registry_bind(registry, name, &wl_shm_interface, 1));
            else if (interface == wl_output_interface.name)
            {
                ; // TODO(v1tr10l7): detected new monitor, we should handle that
                VtCoreInfo("Wayland: Detected monitor: {}", name);
                s_Outputs.push_front(name);
            }
            else if (interface == wl_seat_interface.name)
            {
                static const wl_seat_listener seatListener = {
                    .capabilities = SeatHandleCapabilities,
                    .name         = SeatHandleName,
                };

                s_Seat = reinterpret_cast<wl_seat*>(wl_registry_bind(
                    registry, name, &wl_seat_interface, version));
                wl_seat_add_listener(s_Seat, &seatListener, nullptr);

                if (wl_seat_get_version(s_Seat)
                    >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
                    VtCoreWarn("WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION: {}",
                               WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION);
            }
            else if (interface == wl_data_device_manager_interface.name)
                ;
            else if (interface == xdg_wm_base_interface.name)
            {
                s_WmBase = reinterpret_cast<xdg_wm_base*>(wl_registry_bind(
                    registry, name, &xdg_wm_base_interface, 2));

                auto onWmBasePing = [](void*, xdg_wm_base* wmBase, u32 serial)
                { xdg_wm_base_pong(wmBase, serial); };

                static const xdg_wm_base_listener wmBaseListener
                    = {.ping = onWmBasePing};
                xdg_wm_base_add_listener(s_WmBase, &wmBaseListener, nullptr);
                (void)s_WmBase;
            }
            else if (interface == "zxdg_decoration_manager_v1")
                ;
            else if (interface == "wp_viewporter")
                ;
            else if (interface == "zwp_relative_pointer_manager_v1")
                ;
            else if (interface == "zwp_pointer_constraints_v1")
                ;
            else if (interface == "zwp_idle_inhibit_manager_v1")
                ;
            else if (interface == "xdg_activation_v1")
                ;
            else if (interface == "wp_fractional_scale_manager_v1")
                ;
            else if (interface == wp_alpha_modifier_v1_interface.name)
                s_AlphaModifier
                    = reinterpret_cast<wp_alpha_modifier_v1*>(wl_registry_bind(
                        registry, name, &wp_alpha_modifier_v1_interface, 1));
            else if (interface == zwlr_gamma_control_manager_v1_interface.name)
                s_GammaControlManager
                    = reinterpret_cast<zwlr_gamma_control_manager_v1*>(
                        wl_registry_bind(
                            registry, name,
                            &zwlr_gamma_control_manager_v1_interface, 1));
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

        void PointerHandleEnter(void* userData, wl_pointer* pointer, u32 serial,
                                wl_surface* surface, wl_fixed_t xOffset,
                                wl_fixed_t yOffset)
        {
            if (s_PointerListener && s_PointerListener->enter)
                s_PointerListener->enter(userData, pointer, serial, surface,
                                         xOffset, yOffset);
        }
        void PointerHandleLeave(void* userData, wl_pointer* pointer, u32 serial,
                                wl_surface* surface)
        {
            if (s_PointerListener && s_PointerListener->leave)
                s_PointerListener->leave(userData, pointer, serial, surface);
        }
        void PointerHandleMotion(void* userData, wl_pointer* pointer, u32 time,
                                 wl_fixed_t xOffset, wl_fixed_t yOffset)
        {
            if (s_PointerListener && s_PointerListener->motion)
                s_PointerListener->motion(userData, pointer, time, xOffset,
                                          yOffset);
        }
        void PointerHandleButton(void* userData, wl_pointer* pointer,
                                 u32 serial, u32 time, u32 button, u32 state)
        {
            if (s_PointerListener && s_PointerListener->button)
                s_PointerListener->button(userData, pointer, serial, time,
                                          button, state);
        }
        void PointerHandleAxis(void* userData, wl_pointer* pointer, u32 time,
                               u32 axis, wl_fixed_t value)
        {
            if (s_PointerListener && s_PointerListener->axis)
                s_PointerListener->axis(userData, pointer, time, axis, value);
        }
        void PointerHandleFrame(void* userData, wl_pointer* pointer)
        {
            VT_UNUSED(userData);
            VT_UNUSED(pointer);
        }
        void PointerHandleAxisSource(void* userData, wl_pointer* pointer,
                                     u32 axisSource)
        {
            VT_UNUSED(userData);
            VT_UNUSED(pointer);
            VT_UNUSED(axisSource);
        }
        void PointerHandleAxisStop(void* userData, wl_pointer* pointer,
                                   u32 time, u32 axis)
        {

            VT_UNUSED(userData);
            VT_UNUSED(pointer);
            VT_UNUSED(time);
            VT_UNUSED(axis);
        }
        void PointerHandleAxisDiscrete(void* userData, wl_pointer* pointer,
                                       u32 axis, i32 discrete)
        {
            VT_UNUSED(userData);
            VT_UNUSED(pointer);
            VT_UNUSED(axis);
            VT_UNUSED(discrete);
        }
        void PointerHandleAxisValue120(void* userData, wl_pointer* pointer,
                                       u32 axis, i32 value120)
        {
            VT_UNUSED(userData);
            VT_UNUSED(pointer);
            VT_UNUSED(axis);
            VT_UNUSED(value120);
        }
        void PointerHandleAxisRelativeDirection(void*       userData,
                                                wl_pointer* pointer, u32 axis,
                                                u32 direction)
        {
            VT_UNUSED(userData);
            VT_UNUSED(pointer);
            VT_UNUSED(axis);
            VT_UNUSED(direction);
        }

        void KeyboardHandleKeymap(void* userData, wl_keyboard* keyboard,
                                  u32 format, int fd, u32 size)
        {
            VT_UNUSED(userData);
            VT_UNUSED(keyboard);
            VT_UNUSED(format);
            VT_UNUSED(fd);
            VT_UNUSED(size);
        }
        void KeyboardHandleEnter(void* userData, wl_keyboard* keyboard,
                                 u32 serial, wl_surface* surface,
                                 wl_array* keys)
        {
            if (s_KeyboardListener && s_KeyboardListener->enter)
                s_KeyboardListener->enter(userData, keyboard, serial, surface,
                                          keys);
        }
        void KeyboardHandleLeave(void* userData, wl_keyboard* keyboard,
                                 u32 serial, wl_surface* surface)
        {

            if (s_KeyboardListener && s_KeyboardListener->leave)
                s_KeyboardListener->leave(userData, keyboard, serial, surface);
        }
        void KeyboardHandleKey(void* userData, wl_keyboard* keyboard,
                               u32 serial, u32 time, u32 scancode, u32 state)
        {
            if (s_KeyboardListener && s_KeyboardListener->key)
                s_KeyboardListener->key(userData, keyboard, serial, time,
                                        scancode, state);
        }
        void KeyboardHandleModifiers(void* userData, wl_keyboard* keyboard,
                                     u32 serial, u32 modsDepressed,
                                     u32 modsLatched, u32 modsLocked, u32 group)
        {
            VT_UNUSED(userData);
            VT_UNUSED(keyboard);
            VT_UNUSED(serial);
            VT_UNUSED(modsDepressed);
            VT_UNUSED(modsLatched);
            VT_UNUSED(modsLocked);
            VT_UNUSED(group);
        }
        void KeyboardHandleRepeatInfo(void* userData, wl_keyboard* keyboard,
                                      i32 rate, i32 delay)
        {
            VT_UNUSED(userData);
            VT_UNUSED(keyboard);
            VT_UNUSED(rate);
            VT_UNUSED(delay);
        }

        void SeatHandleCapabilities(void* userData, wl_seat* seat,
                                    u32 capabilities)
        {
            VT_UNUSED(userData);

            static const wl_pointer_listener pointerListener = {
                .enter                   = PointerHandleEnter,
                .leave                   = PointerHandleLeave,
                .motion                  = PointerHandleMotion,
                .button                  = PointerHandleButton,
                .axis                    = PointerHandleAxis,
                .frame                   = PointerHandleFrame,
                .axis_source             = PointerHandleAxisSource,
                .axis_stop               = PointerHandleAxisStop,
                .axis_discrete           = PointerHandleAxisDiscrete,
                .axis_value120           = PointerHandleAxisValue120,
                .axis_relative_direction = PointerHandleAxisRelativeDirection,
            };

            if (capabilities & WL_SEAT_CAPABILITY_POINTER && !s_Pointer)
            {
                s_Pointer = wl_seat_get_pointer(seat);

                wl_pointer_add_listener(s_Pointer, &pointerListener, nullptr);
            }
            else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) && s_Pointer)
            {
                wl_pointer_destroy(s_Pointer);
                s_Pointer = nullptr;
            }

            static const wl_keyboard_listener keyboardListener = {
                .keymap      = KeyboardHandleKeymap,
                .enter       = KeyboardHandleEnter,
                .leave       = KeyboardHandleLeave,
                .key         = KeyboardHandleKey,
                .modifiers   = KeyboardHandleModifiers,
                .repeat_info = KeyboardHandleRepeatInfo,
            };

            if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !s_Keyboard)
            {
                s_Keyboard = wl_seat_get_keyboard(seat);
                wl_keyboard_add_listener(s_Keyboard, &keyboardListener,
                                         nullptr);
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

    void Initialize()
    {
        if (s_Display) return;

        s_Display = wl_display_connect(std::getenv("WAYLAND_DISPLAY"));
        VtCoreAssert(s_Display);

        wl_display_listener displayListener = {
            .error = [](void*, wl_display*, void*, u32, const char* description)
            { VtCoreFatal("Wayland: {}", description); },
            .delete_id = [](void*, wl_display*, u32) {},
        };
        wl_display_add_listener(s_Display, &displayListener, nullptr);

        static const wl_registry_listener registryListener = {
            .global        = RegistryHandleGlobal,
            .global_remove = RegistryHandleGlobalRemove,
        };

        s_Registry = wl_display_get_registry(s_Display);
        VtCoreAssert(s_Registry);
        wl_registry_add_listener(s_Registry, &registryListener, nullptr);

        // Sync so we got all registry objects
        wl_display_roundtrip(s_Display);

        VtCoreAssert(s_Compositor);
        VtCoreAssert(s_Subcompositor);
        VtCoreAssert(s_Shm);
        VtCoreAssert(s_Seat);
    }
    void Shutdown()
    {
        wp_alpha_modifier_v1_destroy(s_AlphaModifier);
        xdg_wm_base_destroy(s_WmBase);
        wl_compositor_destroy(s_Compositor);
        wl_subcompositor_destroy(s_Subcompositor);
        wl_shm_destroy(s_Shm);
        wl_seat_destroy(s_Seat);
        wl_registry_destroy(s_Registry);
        wl_display_disconnect(s_Display);
    }

    wl_display*             GetDisplay() { return s_Display; }
    wl_registry*            GetRegistry() { return s_Registry; }
    wl_compositor*          GetCompositor() { return s_Compositor; }
    wl_subcompositor*       GetSubcompositor() { return s_Subcompositor; }
    wl_shm*                 GetShm() { return s_Shm; }

    std::forward_list<u32>& GetOutputs() { return s_Outputs; }
    wl_seat*                GetSeat() { return s_Seat; }
    xdg_wm_base*            GetWmBase() { return s_WmBase; }
    wp_alpha_modifier_v1*   GetAlphaModifier() { return s_AlphaModifier; }
    zwlr_gamma_control_manager_v1* GetGammaControlManager()
    {
        return s_GammaControlManager;
    }

    void SetPointerListener(wl_pointer_listener* listener)
    {
        s_PointerListener = listener;
    }

    void SetKeyboardListener(wl_keyboard_listener* listener)
    {
        s_KeyboardListener = listener;
    }

}; // namespace Vortex::Wayland
