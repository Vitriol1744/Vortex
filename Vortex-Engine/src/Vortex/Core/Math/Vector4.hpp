//
// Created by vitriol1744 on 16.07.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

#include "Vortex/Core/Math/Vector3.hpp"

#include <ostream>

namespace Vortex::Math
{
    struct Matrix4x4f;
    
    template<typename T>
    struct Vector4
    {
        inline Vector4()                                noexcept = default;
        inline Vector4(T scalar)                        noexcept : x(scalar), y(scalar), z(scalar), w(scalar) { }
        inline Vector4(T x, T y, T z, T w)              noexcept : x(x), y(y), z(z), w(w) { }
        inline Vector4(Vector3<T>& xyz, T w)            noexcept : xyz(xyz), __xyz_w(w) { }
        inline Vector4(Vector2<T>& xy, T z, T w)        noexcept : xy(xy), __xy_z(z), __xy_w(w) { }
        inline Vector4(Vector2<T>& xy, Vector2<T> zw)   noexcept : xy(xy), __xy_z(zw.z), __xy_w(w.z) { }

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

            struct
            {
                Vector2<T> xy;
                T __xy_z;
                T __xy_w;
            };

            struct
            {
                Vector3<T> xyz;
                T __xyz_w;
            };
        };

        inline T Dot(Vector4<T> right) const noexcept
        {
            return x * right.x + y * right.y + z * right.z + w * right.w;
        }

        // Operators
        template<typename T2>
        friend Vector4<T2> operator+(Vector4<T2> left, const Vector4<T2>& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator+(Vector4<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator-(Vector4<T2> left, const Vector4<T2>& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator-(Vector4<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator*(Vector4<T2> left, const Matrix4x4f& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator*(Vector4<T2> left, const Vector4<T2>& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator*(Vector4<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator/(Vector4<T2> left, const Vector4<T2>& right);
        template<typename T2>
        friend Vector4<T2> operator/(Vector4<T2> left, const T2& right);
        template<typename T2>
        friend Vector4<T2> operator%(Vector4<T2> left, const Vector4<T2>& right) noexcept;
        template<typename T2>
        friend Vector4<T2> operator%(Vector4<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend std::ostream& operator<<(std::ostream& os, const Vector4<T2>& right) noexcept;

        inline Vector4<T>& operator+=(Vector4<T>& right) noexcept
        {
            x += right.x;
            y += right.y;
            z += right.z;
            w += right.w;

            return *this;
        }
        inline Vector4<T>& operator+=(T& right) noexcept
        {
            x += right;
            y += right;
            z += right;
            w += right;

            return *this;
        }
        inline Vector4<T>& operator-=(Vector4<T>& right) noexcept
        {
            x -= right.x;
            y -= right.y;
            z -= right.z;
            w -= right.w;

            return *this;
        }
        inline Vector4<T>& operator-=(T& right) noexcept
        {
            x -= right;
            y -= right;
            z -= right;
            w -= right;

            return *this;
        }
        //TODO:inline Vector4<T>& operator*=(Matrix4x4f& right) noexcept
        //{
        //    x = x * right[0 + 0 * 4] + y * right[1 + 0 * 4] + z * right[2 + 0 * 4] + w * right[3 + 0 * 4];
        //    y = x * right[0 + 1 * 4] + y * right[1 + 1 * 4] + z * right[2 + 1 * 4] + w * right[3 + 1 * 4];
        //    z = x * right[0 + 2 * 4] + y * right[1 + 2 * 4] + z * right[2 + 2 * 4] + w * right[3 + 2 * 4];
        //    w = x * right[0 + 3 * 4] + y * right[1 + 3 * 4] + z * right[2 + 3 * 4] + w * right[3 + 3 * 4];
//
        //    return *this;
        //}
        inline Vector4<T>& operator*=(Vector4<T>& right) noexcept
        {
            x *= right.x;
            y *= right.y;
            z *= right.z;
            w *= right.w;

            return *this;
        }
        inline Vector4<T>& operator*=(T& right) noexcept
        {
            x *= right;
            y *= right;
            z *= right;
            w *= right;

            return *this;
        }
        inline Vector4<T>& operator/=(Vector4<T>& right)
        {
            x /= right.x;
            y /= right.y;
            z /= right.z;
            w /= right.w;

            return *this;
        }
        inline Vector4<T>& operator/=(T& right)
        {
            x /= right;
            y /= right;
            z /= right;
            w /= right;

            return *this;
        }
        inline Vector4<T>& operator%=(Vector4<T>& right) noexcept
        {
            x %= right.x;
            y %= right.y;
            z %= right.z;
            w %= right.w;

            return *this;
        }
        inline Vector4<T>& operator%=(T& right) noexcept
        {
            x %= right;
            y %= right;
            z %= right;
            w %= right;

            return *this;
        }
        inline Vector4<T>& operator++() noexcept
        {
            x++;
            y++;
            z++;
            w++;

            return *this;
        }
        inline Vector4<T>& operator++(int) noexcept
        {
            Vector4<T> temp(*this);
            operator++();

            return temp;
        }
        inline Vector4<T>& operator--() noexcept
        {
            x--;
            y--;
            z--;
            w--;

            return *this;
        }
        inline Vector4<T>& operator--(int) noexcept
        {
            Vector4<T> temp(*this);
            operator--();

            return temp;
        }
    };


    using Vec4 = Vector4<float32>;
    using Vec4f = Vector4<float32>;
}