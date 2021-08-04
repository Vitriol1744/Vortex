//
// Created by vitriol1744 on 23.07.2021.
//
#pragma once

#include "Vortex/Core/Math/Vector4.hpp"
#include "Vortex/Core/Math/Matrix.hpp"

// This Header Contains Vector4 friend functions and exists to avoid circular header includes
namespace Vortex::Math
{
    template<typename T>
    Vector4<T> operator+(Vector4<T> left, const Vector4<T>& right) noexcept
    {
        left.x += right.x;
        left.y += right.y;
        left.z += right.z;
        left.w += right.w;
        return left;
    }
    template<typename T>
    Vector4<T> operator+(Vector4<T> left, const T& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector4<T> operator-(Vector4<T> left, const Vector4<T>& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator-(Vector4<T> left, const T& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator*(Vector4<T> left, const Mat4& right) noexcept
    {
        left.x = left.x * right[0 + 0 * 4] + left.y * right[1 + 0 * 4] + left.z * right[2 + 0 * 4] + left.w * right[3 + 0 * 4];
        left.y = left.x * right[0 + 1 * 4] + left.y * right[1 + 1 * 4] + left.z * right[2 + 1 * 4] + left.w * right[3 + 1 * 4];
        left.z = left.x * right[0 + 2 * 4] + left.y * right[1 + 2 * 4] + left.z * right[2 + 2 * 4] + left.w * right[3 + 2 * 4];
        left.w = left.x * right[0 + 3 * 4] + left.y * right[1 + 3 * 4] + left.z * right[2 + 3 * 4] + left.w * right[3 + 3 * 4];
        return left;
    }
    template<typename T>
    Vector4<T> operator*(Vector4<T> left, const Vector4<T>& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator*(Vector4<T> left, const T& right) noexcept
    {
        left.x *= right;
        left.y *= right;
        left.z *= right;
        left.w *= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator/(Vector4<T> left, const Vector4<T>& right)
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator/(Vector4<T> left, const T& right)
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator%(Vector4<T> left, const Vector4<T>& right) noexcept
    {
        left %= right;
        return left;
    }
    template<typename T>
    Vector4<T> operator%(Vector4<T> left, const T& right) noexcept
    {
        left %= right;
        return left;
    }
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Vector4<T>& right) noexcept
    {
        os << "x: " << right.x << " y: " << right.y << " z: " << right.z << " w: " << right.w;
        os << '\n';
        
        return os;
    }
}