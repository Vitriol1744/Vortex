//
// Created by Vitriol1744 on 26.06.2021.
//
#pragma once

#include "Vortex/Core/Types.hpp"
#include "Vortex/Core/Math/Vector4.hpp"

namespace Vortex::Math
{
    template<typename T>
    struct Vector3;
    struct VT_API Matrix4x4f
    {
        inline Matrix4x4f() noexcept = default;
        explicit Matrix4x4f(float32 diagonal) noexcept;
        explicit Matrix4x4f(float32* numbers) noexcept;

        union
        {
            float32 data[4 * 4];
            float32 elements[4][4];
            Vector4<float32> vec[4];
        };

        friend VT_API Matrix4x4f operator*(Matrix4x4f left, const Matrix4x4f& right);
        friend std::ostream& operator<<(std::ostream& os, const Matrix4x4f& right) noexcept;

        Matrix4x4f& Invert();

        inline Matrix4x4f& operator*=(Matrix4x4f& right) noexcept;
        inline Matrix4x4f& operator*=(Matrix4x4f&& right) noexcept;

        inline float32 operator[](int index) const noexcept { return data[index]; }

        static Matrix4x4f Identity();
        static Matrix4x4f Transpose(const Matrix4x4f& matrix) noexcept;

        static Matrix4x4f Translate(const Matrix4x4f& matrix, const Vector3<float32>& translation) noexcept;
        static Matrix4x4f Rotate(const Matrix4x4f& matrix, float32 angle, const Vector3<float32>& rotation) noexcept;
        //TODO: Matrix4x4f: Rotate with Quaternion
        static Matrix4x4f Scale(const Matrix4x4f& matrix, const Vector3<float32>& scale) noexcept;

        static Matrix4x4f Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far) noexcept;
        static Matrix4x4f Perspective(float32 fov, float32 aspectRatio, float32 near, float32 far) noexcept;
        static Matrix4x4f LookAt(const Vector3<float32>& camera, const Vector3<float32>& object, const Vector3<float32>& up);
    };

    using Mat4      = Matrix4x4f;
    using Mat4f     = Matrix4x4f;
    using Mat4x4    = Matrix4x4f;
    using Mat4x4f   = Matrix4x4f;
}