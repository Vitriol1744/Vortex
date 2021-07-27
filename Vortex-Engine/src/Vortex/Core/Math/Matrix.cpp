//
// Created by vitriol1744 on 07.07.2021.
//
#include "Matrix.hpp"

#include "MathCommon.hpp"
#include "Vortex/Core/Math/Vec4.hpp"

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

    Matrix4x4f::Matrix4x4f(float32* numbers) noexcept
    {
        memcpy(numbers, data, sizeof(float) * 16);
    }

    Matrix4x4f& Matrix4x4f::operator*=(Matrix4x4f& right) noexcept
    {
        *this = *this * right;
        return *this;
    }

    Matrix4x4f& Matrix4x4f::operator*=(Matrix4x4f&& right) noexcept
    {
        *this = *this * right;
        return *this;
    }

    Matrix4x4f Matrix4x4f::Translate(const Matrix4x4f& matrix, const Vector3<float32>& translation) noexcept
    {
        Matrix4x4f result = matrix;
        
        result.data[3 + 0 * 4] += translation.x;
        result.data[3 + 1 * 4] += translation.y;
        result.data[3 + 2 * 4] += translation.z;

        return result;
    }

    Matrix4x4f Matrix4x4f::Scale(const Matrix4x4f& matrix, const Vector3<float32>& scale) noexcept
    {
        Matrix4x4f result = matrix;
        
        result.data[0 + 0 * 4] *= scale.x;
        result.data[1 + 1 * 4] *= scale.y;
        result.data[2 + 2 * 4] *= scale.z;

        return result;
    }

    Matrix4x4f Matrix4x4f::Rotate(const Matrix4x4f& matrix, float32 angle, const Vector3<float32>& rotation) noexcept
    {
        //TODO: Refactor this
        Matrix4x4f result(1.0f);
        
        float32 radians = ToRadians(angle);
        float32 cos = Cos(radians);
        float32 sin = Sin(radians);

        if (rotation.x)
        {
            Matrix4x4f xt(1.0f);

            xt.data[0 + 0 * 4] =  1.0f;
            xt.data[1 + 1 * 4] =  cos;
            xt.data[1 + 2 * 4] =  sin;
            xt.data[2 + 1 * 4] = -sin;
            xt.data[2 + 2 * 4] =  cos;


            result = result * xt;
        }
        if (rotation.y)
        {
            Matrix4x4f yt(1.0f);

            yt.data[0 + 0 * 4] =  cos;
            yt.data[0 + 2 * 4] = -sin;
            yt.data[1 + 1 * 4] =  1.0f;
            yt.data[2 + 0 * 4] =  sin;
            yt.data[2 + 2 * 4] =  cos;

            result = result * yt;
        }
        if (rotation.z)
        {
            Matrix4x4f zt(1.0f);

            zt.data[0 + 0 * 4] =  cos;
            zt.data[0 + 1 * 4] =  sin;
            zt.data[1 + 0 * 4] = -sin;
            zt.data[1 + 1 * 4] =  cos;
            zt.data[2 + 2 * 4] =  1.0f;

            result = result * zt;
        }
        result.data[3 + 3 * 4] =  1.0f;

        return result * matrix;
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

    Matrix4x4f Matrix4x4f::LookAt(const Vector3<float32>& camera, const Vector3<float32>& object, const Vector3<float32>& up)
    {
        Matrix4x4f result(1.0f);
        Vector3<float32> f = (Vector3<float32>(object.x - camera.x, object.y - camera.y, object.z - camera.z)).Normalize();
        Vector3<float32> s = f.Cross(up.Normalize());
        Vector3<float32> u = s.Cross(f);

        result.data[0 + 0 * 4] = s.x;
        result.data[0 + 1 * 4] = s.y;
        result.data[0 + 2 * 4] = s.z;

        result.data[1 + 0 * 4] = u.x;
        result.data[1 + 1 * 4] = u.y;
        result.data[1 + 2 * 4] = u.z;

        result.data[2 + 0 * 4] = -f.x;
        result.data[2 + 1 * 4] = -f.y;
        result.data[2 + 2 * 4] = -f.z;

        return Translate(result, Vector3<float32>(-camera.x, -camera.y, -camera.z));
    }

    Matrix4x4f operator*(Matrix4x4f left, const Matrix4x4f& right)
    {
        float32 data[16];
        for (int32 row = 0; row < 4; row++)
        {
            for (int32 col = 0; col < 4; col++)
            {
                float32 sum = 0.0f;
                for (int32 e = 0; e < 4; e++)
                {
                    sum += left.data[e + row * 4] * right.data[col + e * 4];
                }
                data[col + row * 4] = sum;
            }
        }
        memcpy(left.data, data, 4 * 4 * sizeof(float));
        return left;
    }
}