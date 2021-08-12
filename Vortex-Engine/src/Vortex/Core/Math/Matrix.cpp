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

    Matrix4x4f& Matrix4x4f::Invert()
    {
        float32 temp[16];

        temp[0] =   data[5]  * data[10] * data[15] -
                    data[5]  * data[11] * data[14] -
                    data[9]  * data[6]  * data[15] +
                    data[9]  * data[7]  * data[14] +
                    data[13] * data[6]  * data[11] -
                    data[13] * data[7]  * data[10];

        temp[4] =  -data[4]  * data[10] * data[15] +
                    data[4]  * data[11] * data[14] +
                    data[8]  * data[6]  * data[15] -
                    data[8]  * data[7]  * data[14] -
                    data[12] * data[6]  * data[11] +
                    data[12] * data[7]  * data[10];

        temp[8] =   data[4]  * data[9]  * data[15] -
                    data[4]  * data[11] * data[13] -
                    data[8]  * data[5]  * data[15] +
                    data[8]  * data[7]  * data[13] +
                    data[12] * data[5]  * data[11] -
                    data[12] * data[7]  * data[9];

        temp[12] = -data[4]  * data[9]  * data[14] +
                    data[4]  * data[10] * data[13] +
                    data[8]  * data[5]  * data[14] -
                    data[8]  * data[6]  * data[13] -
                    data[12] * data[5]  * data[10] +
                    data[12] * data[6]  * data[9];

        temp[1] =  -data[1]  * data[10] * data[15] +
                    data[1]  * data[11] * data[14] +
                    data[9]  * data[2]  * data[15] -
                    data[9]  * data[3]  * data[14] -
                    data[13] * data[2]  * data[11] +
                    data[13] * data[3]  * data[10];

        temp[5] =   data[0]  * data[10] * data[15] -
                    data[0]  * data[11] * data[14] -
                    data[8]  * data[2]  * data[15] +
                    data[8]  * data[3]  * data[14] +
                    data[12] * data[2]  * data[11] -
                    data[12] * data[3]  * data[10];

        temp[9] =  -data[0]  * data[9]  * data[15] +
                    data[0]  * data[11] * data[13] +
                    data[8]  * data[1]  * data[15] -
                    data[8]  * data[3]  * data[13] -
                    data[12] * data[1]  * data[11] +
                    data[12] * data[3]  * data[9];

        temp[13] =  data[0]  * data[9]  * data[14] -
                    data[0]  * data[10] * data[13] -
                    data[8]  * data[1]  * data[14] +
                    data[8]  * data[2]  * data[13] +
                    data[12] * data[1]  * data[10] -
                    data[12] * data[2]  * data[9];

        temp[2] =   data[1]  * data[6]  * data[15] -
                    data[1]  * data[7]  * data[14] -
                    data[5]  * data[2]  * data[15] +
                    data[5]  * data[3]  * data[14] +
                    data[13] * data[2]  * data[7]  -
                    data[13] * data[3]  * data[6];

        temp[6] =  -data[0]  * data[6]  * data[15] +
                    data[0]  * data[7]  * data[14] +
                    data[4]  * data[2]  * data[15] -
                    data[4]  * data[3]  * data[14] -
                    data[12] * data[2]  * data[7]  +
                    data[12] * data[3]  * data[6];

        temp[10] =  data[0]  * data[5]  * data[15] -
                    data[0]  * data[7]  * data[13] -
                    data[4]  * data[1]  * data[15] +
                    data[4]  * data[3]  * data[13] +
                    data[12] * data[1]  * data[7]  -
                    data[12] * data[3]  * data[5];

        temp[14] = -data[0]  * data[5]  * data[14] +
                    data[0]  * data[6]  * data[13] +
                    data[4]  * data[1]  * data[14] -
                    data[4]  * data[2]  * data[13] -
                    data[12] * data[1]  * data[6]  +
                    data[12] * data[2]  * data[5];

        temp[3] =  -data[1]  * data[6]  * data[11] +
                    data[1]  * data[7]  * data[10] +
                    data[5]  * data[2]  * data[11] -
                    data[5]  * data[3]  * data[10] -
                    data[9]  * data[2]  * data[7]  +
                    data[9]  * data[3]  * data[6];

        temp[7] =   data[0]  * data[6]  * data[11] -
                    data[0]  * data[7]  * data[10] -
                    data[4]  * data[2]  * data[11] +
                    data[4]  * data[3]  * data[10] +
                    data[8]  * data[2]  * data[7]  -
                    data[8]  * data[3]  * data[6];

        temp[11] = -data[0]  * data[5]  * data[11] +
                    data[0]  * data[7]  * data[9]  +
                    data[4]  * data[1]  * data[11] -
                    data[4]  * data[3]  * data[9]  -
                    data[8]  * data[1]  * data[7]  +
                    data[8]  * data[3]  * data[5];

        temp[15] =  data[0]  * data[5]  * data[10] -
                    data[0]  * data[6]  * data[9]  -
                    data[4]  * data[1]  * data[10] +
                    data[4]  * data[2]  * data[9]  +
                    data[8]  * data[1]  * data[6]  -
                    data[8]  * data[2]  * data[5];

        float32 determinant = data[0] * temp[0] + data[1] * temp[4] + data[2] * temp[8] + data[3] * temp[12];
        determinant = 1.0f / determinant;

        for (int32 i = 0; i < 4 * 4; i++) data[i] = temp[i] * determinant;

        return *this;
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

    Matrix4x4f Matrix4x4f::Identity()
    {
        return Matrix4x4f(1.0f);
    }
    Matrix4x4f Matrix4x4f::Transpose(const Matrix4x4f& matrix) noexcept
    {
        Matrix4x4f result(1.0f);

        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j) result.elements[j][i] = matrix.elements[i][j];
        }

        return result;
    }

    Matrix4x4f Matrix4x4f::Translate(const Matrix4x4f& matrix, const Vector3<float32>& translation) noexcept
    {
        Matrix4x4f result = matrix;
        
        result.data[3 + 0 * 4] += translation.x;
        result.data[3 + 1 * 4] += translation.y;
        result.data[3 + 2 * 4] += translation.z;

        return result;
    }
    Matrix4x4f Matrix4x4f::Rotate(const Matrix4x4f& matrix, float32 angle, const Vector3<float32>& rotation) noexcept
    {
        //TODO: To Refactorization:
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
    Matrix4x4f Matrix4x4f::Scale(const Matrix4x4f& matrix, const Vector3<float32>& scale) noexcept
    {
        Matrix4x4f result = matrix;
        
        result.data[0 + 0 * 4] *= scale.x;
        result.data[1 + 1 * 4] *= scale.y;
        result.data[2 + 2 * 4] *= scale.z;

        return result;
    }

    Matrix4x4f Matrix4x4f::Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far) noexcept
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

        result.data[0 + 0 * 4] = q / aspectRatio;
        result.data[1 + 1 * 4] = q;
        result.data[2 + 2 * 4] = (near + far) / (near - far);
        result.data[2 + 3 * 4] = -1.0f;
        result.data[3 + 2 * 4] = (2.0f * near * far) / (near - far);

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

    std::ostream& operator<<(std::ostream& os, const Matrix4x4f& right) noexcept
    {
        os << right.data[0 + 0 * 4] << " " << right.data[1 + 0 * 4] << " " << right.data[2 + 0 * 4] << " " << right.data[3 + 0 * 4] << "\n";
        os << right.data[0 + 1 * 4] << " " << right.data[1 + 1 * 4] << " " << right.data[2 + 1 * 4] << " " << right.data[3 + 1 * 4] << "\n";
        os << right.data[0 + 2 * 4] << " " << right.data[1 + 2 * 4] << " " << right.data[2 + 2 * 4] << " " << right.data[3 + 2 * 4] << "\n";
        os << right.data[0 + 3 * 4] << " " << right.data[1 + 3 * 4] << " " << right.data[2 + 3 * 4] << " " << right.data[3 + 3 * 4] << "\n";

        return os;
    }
}