/*
 * Created by v1tr10l7 on 29.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Math/Vector.hpp>

namespace Vortex::Math
{
    struct AABB
    {
        Vec3 Min;
        Vec3 Max;

        AABB()
            : Min(Vec3(0))
            , Max(Vec3(0))
        {
        }

        AABB(const Vec3& min, const Vec3& max)
            : Min(min)
            , Max(max)
        {
        }
    };
}; // namespace Vortex::Math
