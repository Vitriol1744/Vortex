//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Core/Math/Vector.hpp"

namespace Vortex::Math
{
    struct Matrix4x4f
    {
        inline Matrix4x4f() noexcept = default;
        explicit Matrix4x4f(float32 diagonal) noexcept;

        union
        {
            float32 data[4 * 4];
            float32 elements[4][4];
            Vector4<float32> vec[4];
        };

        static Matrix4x4f Translate(Math::Vec3 translation) noexcept;
        static Matrix4x4f Scale(Math::Vec3 scale) noexcept;
        static Matrix4x4f Rotate(float32 angle, Math::Vec3 rotation) noexcept;

        static Matrix4x4f Orthographic(float32 left, float32 right, float32 top, float32 bottom, float32 near, float32 far) noexcept;
        static Matrix4x4f Perspective(float32 fov, float32 aspectRatio, float32 near, float32 far) noexcept;
    };

    using Mat4    = Matrix4x4f;
    using Mat4f   = Matrix4x4f;
    using Mat4x4 = Matrix4x4f;
    using Mat4x4f = Matrix4x4f;
}