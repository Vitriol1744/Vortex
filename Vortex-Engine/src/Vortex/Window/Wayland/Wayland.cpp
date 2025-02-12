/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Window/Wayland/Wayland.hpp"
#include "Vortex/Window/Wayland/WaylandMonitor.hpp"

#include <sys/mman.h>
#include <unistd.h>

namespace Vortex::Wayland
{
    namespace
    {
        // Core wayland structures
        wl_display*                    s_Display       = nullptr;
        wl_registry*                   s_Registry      = nullptr;
        wl_compositor*                 s_Compositor    = nullptr;
        wl_subcompositor*              s_Subcompositor = nullptr;
        wl_shm*                        s_Shm           = nullptr;

        wl_seat*                       s_Seat          = nullptr;
        wl_pointer*                    s_Pointer       = nullptr;
        wl_keyboard*                   s_Keyboard      = nullptr;

        // Xkb stuff
        XkbData                        s_XkbData{};

        // Wayland extensions
        xdg_wm_base*                   s_WmBase              = nullptr;
        xdg_toplevel_icon_manager_v1*  s_IconManager         = nullptr;
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
            VtUnused(userData);

            std::string_view interface = interfaceName;
            VtCoreTrace("Wayland: Registry global: {}", interface);

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
                VtCoreInfo("Wayland: Detected monitor: {}", name);
                WaylandMonitor::Connect(name, 2);
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
            }
            else if (interface == xdg_toplevel_icon_v1_interface.name)
                s_IconManager = reinterpret_cast<xdg_toplevel_icon_manager_v1*>(
                    wl_registry_bind(registry, name,
                                     &xdg_toplevel_icon_v1_interface, 1));
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
        }
        void RegistryHandleGlobalRemove(void* userData, wl_registry* registry,
                                        u32 name)
        {
            VtUnused(userData);
            VtUnused(registry);

            WaylandMonitor::Disconnect(name);
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
            VtUnused(userData);
            VtUnused(pointer);
        }
        void PointerHandleAxisSource(void* userData, wl_pointer* pointer,
                                     u32 axisSource)
        {
            VtUnused(userData);
            VtUnused(pointer);
            VtUnused(axisSource);
        }
        void PointerHandleAxisStop(void* userData, wl_pointer* pointer,
                                   u32 time, u32 axis)
        {

            VtUnused(userData);
            VtUnused(pointer);
            VtUnused(time);
            VtUnused(axis);
        }
        void PointerHandleAxisDiscrete(void* userData, wl_pointer* pointer,
                                       u32 axis, i32 discrete)
        {
            VtUnused(userData);
            VtUnused(pointer);
            VtUnused(axis);
            VtUnused(discrete);
        }
        void PointerHandleAxisValue120(void* userData, wl_pointer* pointer,
                                       u32 axis, i32 value120)
        {
            VtUnused(userData);
            VtUnused(pointer);
            VtUnused(axis);
            VtUnused(value120);
        }
        void PointerHandleAxisRelativeDirection(void*       userData,
                                                wl_pointer* pointer, u32 axis,
                                                u32 direction)
        {
            VtUnused(userData);
            VtUnused(pointer);
            VtUnused(axis);
            VtUnused(direction);
        }

        void KeyboardHandleKeymap(void* userData, wl_keyboard* keyboard,
                                  u32 format, int fd, u32 size)
        {
            VtUnused(userData);
            VtUnused(keyboard);

            if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
            {
                close(fd);
                return;
            }

            char* mapStr = reinterpret_cast<char*>(
                mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
            if (mapStr == MAP_FAILED)
            {
                close(fd);
                return;
            }

            xkb_keymap* keymap = xkb_keymap_new_from_string(
                s_XkbData.Context, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1,
                xkb_keymap_compile_flags(0));
            munmap(mapStr, size);
            close(fd);

            VtCoreAssert(keymap);
            xkb_state* state = xkb_state_new(keymap);
            if (!state)
            {
                VtCoreAssertMsg(s_XkbData.State,
                                "Wayland: Failed to create XKB state");
                xkb_keymap_unref(keymap);
                return;
            }

            const char* locale = std::getenv("LC_ALL");
            if (!locale) locale = std::getenv("LC_CTYPE");
            if (!locale) locale = std::getenv("LANG");
            if (!locale) locale = "C";

            xkb_compose_table* composeTable = xkb_compose_table_new_from_locale(
                s_XkbData.Context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);

            xkb_compose_state* composeState = xkb_compose_state_new(
                composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
            xkb_compose_table_unref(composeTable);
            s_XkbData.ComposeState = composeState;

            xkb_keymap_unref(s_XkbData.Keymap);
            xkb_state_unref(s_XkbData.State);
            s_XkbData.Keymap = keymap;
            s_XkbData.State  = state;

            s_XkbData.ControlIndex
                = xkb_keymap_mod_get_index(s_XkbData.Keymap, "Control");
            s_XkbData.AltIndex
                = xkb_keymap_mod_get_index(s_XkbData.Keymap, "Mod1");
            s_XkbData.ShiftIndex
                = xkb_keymap_mod_get_index(s_XkbData.Keymap, "Shift");
            s_XkbData.SuperIndex
                = xkb_keymap_mod_get_index(s_XkbData.Keymap, "Mod4");
            s_XkbData.CapsLockIndex
                = xkb_keymap_mod_get_index(s_XkbData.Keymap, "Lock");
            s_XkbData.NumLockIndex
                = xkb_keymap_mod_get_index(s_XkbData.Keymap, "Mod2");
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
            VtUnused(userData);
            VtUnused(keyboard);
            VtUnused(serial);

            if (!s_XkbData.Keymap) return;
            xkb_state_update_mask(s_XkbData.State, modsDepressed, modsLatched,
                                  modsLocked, 0, 0, group);

            s_XkbData.Modifiers = 0;
        }
        void KeyboardHandleRepeatInfo(void* userData, wl_keyboard* keyboard,
                                      i32 rate, i32 delay)
        {
            VtUnused(userData);
            VtUnused(keyboard);
            VtUnused(rate);
            VtUnused(delay);
        }

        void SeatHandleCapabilities(void* userData, wl_seat* seat,
                                    u32 capabilities)
        {
            VtUnused(userData);

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
            VtUnused(name);
            VtCoreInfo("Wayland: Bound to seat: '{}'", name);
        }
    }; // namespace

    void Initialize()
    {
        if (s_Display) return;

        s_Display = wl_display_connect(std::getenv("WAYLAND_DISPLAY"));
        VtCoreAssert(s_Display);

        wl_display_listener displayListener = {
            .error =
                [](void*, wl_display*, void*, u32, const char* description)
            {
                (void)description;
                VtCoreFatal("Wayland: {}", description);
            },
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

        // Sync so we got all initial output events
        wl_display_roundtrip(s_Display);

        VtCoreAssert(s_Compositor);
        VtCoreAssert(s_Subcompositor);
        VtCoreAssert(s_Shm);
        VtCoreAssert(s_Seat);
        VtCoreAssert(s_WmBase);

        s_XkbData.Context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
        VtCoreAssertMsg(s_XkbData.Context,
                        "Wayland: Failed to initialize xkb context");
    }
    void Shutdown()
    {
        WaylandMonitor::Shutdown();
        xkb_context_unref(s_XkbData.Context);

        if (s_GammaControlManager)
            zwlr_gamma_control_manager_v1_destroy(s_GammaControlManager);
        if (s_AlphaModifier) wp_alpha_modifier_v1_destroy(s_AlphaModifier);
        if (s_IconManager) xdg_toplevel_icon_manager_v1_destroy(s_IconManager);
        xdg_wm_base_destroy(s_WmBase);
        wl_compositor_destroy(s_Compositor);
        wl_subcompositor_destroy(s_Subcompositor);
        wl_shm_destroy(s_Shm);
        if (s_Keyboard) wl_keyboard_destroy(s_Keyboard);
        s_Keyboard = nullptr;
        if (s_Pointer) wl_pointer_destroy(s_Pointer);
        s_Pointer = nullptr;
        wl_seat_destroy(s_Seat);
        wl_registry_destroy(s_Registry);
        wl_display_disconnect(s_Display);
        s_Display = nullptr;
    }

    wl_display*       GetDisplay() { return s_Display; }
    wl_registry*      GetRegistry() { return s_Registry; }
    wl_compositor*    GetCompositor() { return s_Compositor; }
    wl_subcompositor* GetSubcompositor() { return s_Subcompositor; }
    wl_shm*           GetShm() { return s_Shm; }

    wl_seat*          GetSeat() { return s_Seat; }
    const XkbData&    GetXkbData() { return s_XkbData; }
    xkb_keysym_t      ComposeSymbol(xkb_keysym_t symbol)
    {
        if (symbol == XKB_KEY_NoSymbol || !s_XkbData.ComposeState)
            return symbol;
        if (xkb_compose_state_feed(s_XkbData.ComposeState, symbol)
            != XKB_COMPOSE_FEED_ACCEPTED)
            return symbol;

        switch (xkb_compose_state_get_status(s_XkbData.ComposeState))
        {
            case XKB_COMPOSE_COMPOSED:
                return xkb_compose_state_get_one_sym(s_XkbData.ComposeState);
            case XKB_COMPOSE_COMPOSING:
            case XKB_COMPOSE_CANCELLED: return XKB_KEY_NoSymbol;
            case XKB_COMPOSE_NOTHING:
            default: return symbol;
        }
    }

    xdg_wm_base*                  GetWmBase() { return s_WmBase; }
    xdg_toplevel_icon_manager_v1* GetIconManager() { return s_IconManager; }
    wp_alpha_modifier_v1*         GetAlphaModifier() { return s_AlphaModifier; }
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
