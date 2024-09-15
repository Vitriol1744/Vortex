/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Math/Matrix.hpp"
#include "Vortex/Core/Math/Quaternion.hpp"
#include "Vortex/Core/Math/Vector.hpp"

namespace Vortex
{
    template <typename T>
    inline consteval T Pow(T base, T exponent)
    {
        T pow = base;
        for (T i = 1; i < exponent; i++) pow *= base;

        return pow;
    }
}; // namespace Vortex
