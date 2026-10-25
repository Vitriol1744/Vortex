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
    }; // namespace Mouse
}; // namespace Vortex
