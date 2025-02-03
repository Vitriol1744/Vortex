/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Types.hpp"

namespace Vortex::Input
{
    enum class MouseCode : i8
    {
        eUnknown = -1,
        eLeft,
        eRight,
        eMiddle,
        eX1,
        eX2,
        eX3,
        eX4,
        eX5,

        eButtonCount
    };
}
