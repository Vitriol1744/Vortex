//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Export.hpp"
#include "Core/Types.hpp"

namespace Vortex::Math
{
    template<typename T>
    struct VT_API Vector4
    {
        Vector4() = default;
        inline Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) { }

        union
        {
            T v[4];
            struct
            {
                T x, y, z, w;
            };
            struct
            {
                T r, g, b, a;
            };
        };
    };

    template<typename T>
    struct VT_API Vector3
    {
        Vector3() = default;
        inline Vector3(T x, T y, T z) : x(x), y(y), z(z) { }

        union
        {
            T v[3];
            struct
            {
                T x, y, z;
            };
            struct
            {
                T r, g, b;
            };
        };
    };

    template<typename T>
    struct VT_API Vector2
    {
        Vector2() = default;
        inline Vector2(T x, T y) : x(x), y(y) { }

        union
        {
            T v[2];
            struct
            {
                T x, y;
            };
            struct
            {
                T r, g;
            };
        };
    };

    using Vec4 = Vector4<float32>;
    using Vec3 = Vector3<float32>;
    using Vec2 = Vector2<float32>;
}