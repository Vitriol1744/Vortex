//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_MATH_HPP
#define VORTEX_MATH_HPP

#include "Vortex/Core/Math/MathCommon.hpp"
#include "Vortex/Core/Math/Rectangle.hpp"

#undef near
#undef far

namespace Vortex::Math
{
    using Matrix4       = glm::mat4;
    using Matrix3       = glm::mat4;
    using Matrix2       = glm::mat2;
    template<uint16 columns, uint16 rows, typename T>
    using Matrix        = glm::mat<columns, rows, T>;

    using Mat4          = Matrix4;
    using Mat3          = Matrix3;
    using Mat2          = Matrix2;

    using Vector4       = glm::vec<4, float32>;
    using Vector3       = glm::vec<3, float32>;
    using Vector2       = glm::vec<2, float32>;
    template<uint16 count, typename T>
    using Vector        = glm::vec<count, T>;

    using Vec4          = Vector4;
    using Vec3          = Vector3;
    using Vec2          = Vector2;
    using Vec2f         = Vector2;
    using Vec2d         = Vector<2, float64>;
    using Vec2i         = Vector<2, int32>;
    using Vec2u         = Vector<2, uint32>;

    using Quaternion    = glm::quat;
    using Quat          = Quaternion;

    inline Vec3 Rotate(Quaternion quat, Vec3 vec) { return glm::rotate(quat, vec); }

    inline Mat4 Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far)
    {
        return glm::ortho(left, right, bottom, top, near, far);
    }
    inline Mat4 Perspective(float32 fov, float32 aspect_ratio, float32 near, float32 far)
    {
        return glm::perspective(fov, aspect_ratio, near, far);
    }

    inline Mat4 Translate(Mat4 mat, Vec3 translation)
    {
        return glm::translate(mat, translation);
    }
    inline Mat4 Rotate(Mat4 mat, float32 angle, Vec3 rotation)
    {
        return glm::rotate(mat, static_cast<float32>(ToRadians(angle)), rotation);
    }
    inline Mat4 Scale(Mat4 mat, Vec3 scale) { return glm::scale(mat, scale); }

    inline Mat4 ToMat4(Quaternion quat) { return glm::mat4_cast(quat); }

    inline Quaternion AngleAxis(float32 angle, Vec3 axis)
    {
        return glm::angleAxis(angle, axis);
    }
    inline Quaternion Normalize(Quaternion quat)
    {
        return glm::normalize(quat);
    }
    inline Quaternion Inverse(Quaternion quat) { return glm::inverse(quat); }
}

#endif //VORTEX_MATH_HPP
