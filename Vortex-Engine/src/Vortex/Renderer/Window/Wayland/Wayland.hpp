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
#include <wayland-xdg-shell-client-protocol.h>

namespace Vortex
{
    namespace Wayland
    {
        void                  Initialize();
        void                  Shutdown();

        wl_display*           GetDisplay();
        wl_registry*          GetRegistry();
        wl_compositor*        GetCompositor();
        wl_subcompositor*     GetSubcompositor();
        wl_shm*               GetShm();
        wl_seat*              GetSeat();
        xdg_wm_base*          GetWmBase();
        wp_alpha_modifier_v1* GetAlphaModifier();

        void                  SetPointerListener(wl_pointer_listener* listener);
        void SetKeyboardListener(wl_keyboard_listener* listener);
    }; // namespace Wayland
};     // namespace Vortex
