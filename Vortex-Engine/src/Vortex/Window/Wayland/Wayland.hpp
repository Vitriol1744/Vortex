/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

#include <wayland-alpha-modifier-v1-client-protocol.h>
#include <wayland-wayland-client-protocol.h>
#include <wayland-wlr-gamma-control-unstable-v1-client-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>
#include <wayland-xdg-toplevel-icon-v1-client-protocol.h>

#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon.h>

namespace Vortex
{
    namespace Wayland
    {
        struct XkbData
        {
            xkb_context*       Context       = nullptr;
            xkb_keymap*        Keymap        = nullptr;
            xkb_state*         State         = nullptr;
            xkb_compose_state* ComposeState  = nullptr;
            xkb_mod_index_t    ControlIndex  = 0;
            xkb_mod_index_t    AltIndex      = 0;
            xkb_mod_index_t    ShiftIndex    = 0;
            xkb_mod_index_t    SuperIndex    = 0;
            xkb_mod_index_t    CapsLockIndex = 0;
            xkb_mod_index_t    NumLockIndex  = 0;

            u32                Modifiers     = 0;
        };

        void                           Initialize();
        void                           Shutdown();

        wl_display*                    GetDisplay();
        wl_registry*                   GetRegistry();
        wl_compositor*                 GetCompositor();
        wl_subcompositor*              GetSubcompositor();
        wl_shm*                        GetShm();

        wl_seat*                       GetSeat();
        const XkbData&                 GetXkbData();
        xkb_keysym_t                   ComposeSymbol(xkb_keysym_t symbol);

        xdg_wm_base*                   GetWmBase();
        xdg_toplevel_icon_manager_v1*  GetIconManager();
        wp_alpha_modifier_v1*          GetAlphaModifier();
        zwlr_gamma_control_manager_v1* GetGammaControlManager();

        void SetPointerListener(wl_pointer_listener* listener);
        void SetKeyboardListener(wl_keyboard_listener* listener);
    }; // namespace Wayland
};     // namespace Vortex
