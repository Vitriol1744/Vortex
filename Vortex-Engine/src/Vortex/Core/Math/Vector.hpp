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
    using Vector4 = glm::vec<4, f32>;
    using Vector3 = glm::vec<3, f32>;
    using Vector2 = glm::vec<2, f32>;
    template <u16 count, typename T>
    using Vector = glm::vec<count, T>;

    using Vec4   = Vector4;
    using Vec3   = Vector3;
    using Vec2   = Vector2;
    using Vec2f  = Vector2;
    using Vec2d  = Vector<2, f64>;
    using Vec2i  = Vector<2, i32>;
    using Vec2u  = Vector<2, u32>;

}; // namespace Vortex
