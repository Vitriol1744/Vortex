//
// Created by vitriol1744 on 16.07.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

#include "Vortex/Core/Math/MathCommon.hpp"
#include "Vortex/Core/Math/Vector2.hpp"

#include <ostream>

namespace Vortex::Math
{
    template<typename T>
    struct Vector3
    {
        inline Vector3() noexcept = default;
        inline Vector3(T scalar) noexcept : x(scalar), y(scalar), z(scalar) { }
        inline Vector3(T x, T y, T z) noexcept : x(x), y(y), z(z) { }
        inline Vector3(Vector2<T> xy, T z) noexcept : xy(xy), __xy_z(z) { }

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

            struct
            {
                Vector2<T> xy;
                T __xy_z;
            };
        };

        inline Vector3<T> Cross(const Vector3<T>& right) const
        {
            return Vector3<T>(y * right.z - z * right.y, z * right.x - x * right.z, x * right.y - y * right.x);
        }

        inline T Dot(Vector3<T> right) const noexcept
        {
            return x * right.x + y * right.y + z * right.z;
        }

        inline float Magnitude() const
        {
            return Sqrt(x * x + y * y + z * z);
        }

        inline Vector3<T> Normalize() const
        {
            float length = Magnitude();
            return Vector3<T>(x / length, y / length, z / length);
        }

        // Operators
        template<typename T2>
        friend Vector3<T2> operator+(Vector3<T2> left, const Vector3<T2>& right) noexcept;
        template<typename T2>
        friend Vector3<T2> operator+(Vector3<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector3<T2> operator-(Vector3<T2> left, const Vector3<T2>& right) noexcept;
        template<typename T2>
        friend Vector3<T2> operator-(Vector3<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector3<T2> operator*(Vector3<T2> left, const Vector3<T2>& right) noexcept;
        template<typename T2>
        friend Vector3<T2> operator*(Vector3<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector3<T2> operator/(Vector3<T2> left, const Vector3<T2>& right);
        template<typename T2>
        friend Vector3<T2> operator/(Vector3<T2> left, const T2& right);
        template<typename T2>
        friend Vector3<T2> operator%(Vector3<T2> left, const Vector3<T2>& right);
        template<typename T2>
        friend Vector3<T2> operator%(Vector3<T2> left, const T2& right);
        template<typename T2>
        friend std::ostream& operator<<(std::ostream& os, const Vector3<T2>& right) noexcept;

        inline Vector3<T>& operator+=(Vector3<T>& right) noexcept
        {
            x += right.x;
            y += right.y;
            z += right.z;

            return *this;
        }

        inline Vector3<T>& operator+=(T& right) noexcept
        {
            x += right;
            y += right;
            z += right;

            return *this;
        }

        inline Vector3<T>& operator-=(Vector3<T>& right) noexcept
        {
            x -= right.x;
            y -= right.y;
            z -= right.z;

            return *this;
        }

        inline Vector3<T>& operator-=(T& right) noexcept
        {
            x -= right;
            y -= right;
            z -= right;

            return *this;
        }

        inline Vector3<T>& operator*=(Vector3<T>& right) noexcept
        {
            x *= right.x;
            y *= right.y;
            z *= right.z;

            return *this;
        }

        inline Vector3<T>& operator*=(T& right) noexcept
        {
            x *= right;
            y *= right;
            z *= right;

            return *this;
        }

        inline Vector3<T>& operator/=(Vector3<T>& right) noexcept
        {
            x /= right.x;
            y /= right.y;
            z /= right.z;

            return *this;
        }

        inline Vector3<T>& operator/=(T& right) noexcept
        {
            x /= right;
            y /= right;
            z /= right;

            return *this;
        }

        inline Vector3<T>& operator%=(Vector3<T>& right)
        {
            x %= right.x;
            y %= right.y;
            z %= right.z;

            return *this;
        }

        inline Vector3<T>& operator%=(T& right)
        {
            x %= right;
            y %= right;
            z %= right;

            return *this;
        }

        inline Vector3<T>& operator++() noexcept
        {
            x++;
            y++;
            z++;

            return *this;
        }

        inline Vector3<T>& operator++(int) noexcept
        {
            Vector3<T> temp(*this);
            operator++();

            return temp;
        }

        inline Vector3<T>& operator--() noexcept
        {
            x--;
            y--;
            z--;

            return *this;
        }

        inline Vector3<T>& operator--(int) noexcept
        {
            Vector3<T> temp(*this);
            operator--();

            return temp;
        }
    };

    template<typename T>
    Vector3<T> operator+(Vector3<T> left, const Vector3<T>& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector3<T> operator+(Vector3<T> left, const T& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector3<T> operator-(Vector3<T> left, const Vector3<T>& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator-(Vector3<T> left, const T& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator*(Vector3<T> left, const Vector3<T>& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator*(Vector3<T> left, const T& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator/(Vector3<T> left, const Vector3<T>& right)
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator/(Vector3<T> left, const T& right)
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator%(Vector3<T> left, const Vector3<T>& right)
    {
        left %= right;
        return left;
    }
    template<typename T>
    Vector3<T> operator%(Vector3<T> left, const T& right)
    {
        left %= right;
        return left;
    }
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Vector3<T>& right) noexcept
    {
        os << "x: " << right.x << " y: " << right.y << " z: " << right.z;
        os << '\n';

        return os;
    }


    using Vec3 = Vector3<float32>;
    using Vec3f = Vector3<float32>;
}