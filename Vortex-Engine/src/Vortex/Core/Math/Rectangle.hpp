/*
 * Created by v1tr10l7 on 27.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    template <typename T>
    concept arithmetic = std::integral<T> or std::floating_point<T>;

    template <typename T = i32>
        requires arithmetic<T>
    struct Rectangle
    {
        Rectangle() = default;
        Rectangle(T x, T y, T width, T height)
            : X(x)
            , Y(y)
            , Width(width)
            , Height(height)
        {
        }
        template <typename U>
        explicit Rectangle(const Rectangle<U>& other)
            : X(static_cast<T>(other.X))
            , Y(static_cast<T>(other.Y))
            , Width(static_cast<T>(other.Width))
            , Height(static_cast<T>(other.Height))
        {
        }

        T X      = 0;
        T Y      = 0;
        T Width  = 0;
        T Height = 0;
    };

    template <typename T>
    inline bool operator==(const Rectangle<T>& left, const Rectangle<T>& right)
    {
        return left.X == right.X && left.Y == right.Y
            && left.Width == right.Width && left.Height == right.Height;
    }
    template <typename T>
    inline bool operator!=(const Rectangle<T>& left, const Rectangle<T>& right)
    {
        return left.X != right.X && left.Y != right.Y
            && left.Width != right.Width && left.Height != right.Height;
    }
}; // namespace Vortex
