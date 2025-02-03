/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Types.hpp"

#include <glm/glm.hpp>

namespace Vortex
{
    template <usize N, typename T = f32>
    struct Vector;

    template <typename T>
    struct Vector<4, T>
    {
        Vector()
            : x(0)
            , y(0)
            , z(0)
            , w(0)
        {
        }

        union
        {
            T x, r, s;
        };
        union
        {
            T y, g, t;
        };
        union
        {
            T z, b, p;
        };
        union
        {
            T w, a, q;
        };
    };

    using Vec4  = glm::vec<4, f32>;
    using Vec4i = glm::vec<4, i32>;
    using Vec3  = glm::vec<3, f32>;
    using Vec2  = glm::vec<2, f32>;
    using Vec2f = glm::vec<2, f32>;
    using Vec2d = glm::vec<2, f64>;
    using Vec2i = glm::vec<2, i32>;
    using Vec2u = glm::vec<2, u32>;
}; // namespace Vortex
