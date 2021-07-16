//
// Created by vitriol1744 on 07.07.2021.
//
#include "Matrix.hpp"

#include "MathCommon.hpp"
#include "Core/Math/Vector.hpp"

#include <cstring>

namespace Vortex::Math
{
    Matrix4x4f::Matrix4x4f(float32 diagonal) noexcept
    {
        memset(data, 0, 4 * 4 * sizeof(float32));

        data[0 + 0 * 4] = diagonal;
        data[1 + 1 * 4] = diagonal;
        data[2 + 2 * 4] = diagonal;
        data[3 + 3 * 4] = diagonal;
    }

    Matrix4x4f Matrix4x4f::Translate(Vector3<float32> translation) noexcept
    {
        Matrix4x4f result(1.0f);

        result.data[3 + 0 * 4] = translation.x;
        result.data[3 + 1 * 4] = translation.y;
        result.data[3 + 2 * 4] = translation.z;

        return result;
    }

    Matrix4x4f Matrix4x4f::Scale(Vector3<float32> scale) noexcept
    {
        Matrix4x4f result(1.0f);

        result.data[0 + 0 * 4] = scale.x;
        result.data[1 + 1 * 4] = scale.y;
        result.data[2 + 2 * 4] = scale.z;

        return result;
    }

    Matrix4x4f Matrix4x4f::Rotate(float32 angle, Vector3<float32> rotation) noexcept
    {
        Matrix4x4f result(1.0f);

        //TODO: Rotate matrix before returning!

        return result;
    }

    Matrix4x4f Matrix4x4f::Orthographic(float32 left, float32 right, float32 top, float32 bottom, float32 near, float32 far) noexcept
    {
        Mat4 result(1.0f);

        result.data[0 + 0 * 4] = 2.0f / (right - left);
        result.data[1 + 1 * 4] = 2.0f / (top - bottom);
        result.data[2 + 2 * 4] = 2.0f / (near - far);

        result.data[3 + 0 * 4] = (left + right) / (left - right);
        result.data[3 + 1 * 4] = (bottom + top) / (bottom - top);
        result.data[3 + 2 * 4] = (far + near) / (far - near);

        return result;
    }

    Matrix4x4f Matrix4x4f::Perspective(float32 fov, float32 aspectRatio, float32 near, float32 far) noexcept
    {
        Matrix4x4f result(1.0f);

        float32 q = 1.0f / (float32)Tan(ToRadians(0.5f * fov));

        result.data[0 + 0 * 4] = q/aspectRatio;
        result.data[1 + 1 * 4] = q;
        result.data[2 + 2 * 4] = (near+far)/(near-far);
        result.data[2 + 3 * 4] = -1.0f;
        result.data[3 + 2 * 4] = (2.0f*near*far)/(near-far);

        return result;
    }
}