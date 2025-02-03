/*
 * Created by v1tr10l7 on 21.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Timestep.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    namespace Time
    {
        Timestep GetCurrentTime();
        void     Sleep(Timestep timestep);
    }; // namespace Time
};     // namespace Vortex
