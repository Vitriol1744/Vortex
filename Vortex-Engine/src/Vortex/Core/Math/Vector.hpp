//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Export.hpp"
#include "Core/Types.hpp"

namespace Vortex
{
    struct VT_API Vector4f
    {
        inline Vector4f(float32 x, float32 y, float32 z, float32 w) : x(x), y(y), z(z), w(w) { }

        union
        {
            float32 v[4];
            struct
            {
                float32 x, y, z, w;
            };
            struct
            {
                float32 r, g, b, a;
            };
        };
    };

    using Vec4 = Vector4f;
}