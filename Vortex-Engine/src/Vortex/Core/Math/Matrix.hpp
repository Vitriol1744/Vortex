//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Math/Vector.hpp"

namespace Vortex::Math
{
    struct Matrix4x4f
    {
        inline Matrix4x4f() = default;
        explicit Matrix4x4f(float diagonal);

        union
        {
            float data[4 * 4];
            float elements[4][4];
            Vec4 vec[4];
        };

        static Matrix4x4f Translate(Math::Vec3 translation);

        static Matrix4x4f Orthographic(float left, float right, float top, float bottom, float near, float far);
        static Matrix4x4f Perspective(float fov, float aspectRatio, float near, float far);
    };

    using Mat4    = Matrix4x4f;
    using Mat4f   = Matrix4x4f;
    using Mat4x4f = Matrix4x4f;
}