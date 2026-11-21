/*
 * Created by v1tr10l7 on 18.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Events/EventBus.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Window/Export.hpp>
#include <Vortex/Window/Input/MouseCode.hpp>

namespace Vortex
{
    namespace Mouse
    {
        void VT_WINDOW_API        Initialize(EventBus* eventBus);

        bool VT_WINDOW_API        IsButtonPressed(Input::MouseCode mouseCode);
        Math::Vec2d VT_WINDOW_API Position();
        /// Mouse motion since the last call, in pixels (raw, unaccelerated
        /// where the platform reports it, e.g. a locked Wayland pointer).
        Math::Vec2d VT_WINDOW_API ConsumeDelta();
        /// Backends with raw motion report it here.
        void VT_WINDOW_API        AddRawDelta(f64 dx, f64 dy);
    }; // namespace Mouse
}; // namespace Vortex
