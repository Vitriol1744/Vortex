
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
    using Matrix4 = glm::mat4;
    using Matrix3 = glm::mat4;
    using Matrix2 = glm::mat2;
    template <u16 columns, u16 rows, typename T>
    using Matrix = glm::mat<columns, rows, T>;

    using Mat4   = Matrix4;
    using Mat3   = Matrix3;
    using Mat2   = Matrix2;
}; // namespace Vortex
