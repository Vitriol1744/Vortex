/*
 * Created by v1tr10l7 on 18.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Math/Vector.hpp"
#include "Vortex/Window/Input/MouseCode.hpp"

namespace Vortex
{
    namespace Mouse
    {
        void  Initialize();

        bool  IsButtonPressed(Input::MouseCode mouseCode);
        Vec2d GetPosition();
    }; // namespace Mouse
};     // namespace Vortex
