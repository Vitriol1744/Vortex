//
// Created by vitriol1744 on 16.07.2021.
//
#pragma once

#include "Core/Export.hpp"
#include "Core/Types.hpp"

#include "Core/Math/Vector2.hpp"

#include <ostream>

namespace Vortex::Math
{
    template<typename T>
    struct VT_API Vector3
    {
        inline Vector3() noexcept = default;
        inline Vector3(T scalar) noexcept : x(scalar), y(scalar), z(scalar) { }
        inline Vector3(T x, T y, T z) noexcept : x(x), y(y), z(z) { }
        inline Vector3(Vector2<T>& xy, T z) noexcept : xy(xy), __xy_z(z) { }

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

        inline T Dot(Vector3<T> right) const noexcept
        {
            return x * right.x + y * right.y + z * right.z;
        }

        // Operators
        friend Vector3<T>& operator+(Vector3<T>& left, const Vector3<T>& right) noexcept;
        friend Vector3<T>& operator+(Vector3<T>& left, const T& right) noexcept;
        friend Vector3<T>& operator-(Vector3<T>& left, const Vector3<T>& right) noexcept;
        friend Vector3<T>& operator-(Vector3<T>& left, const T& right) noexcept;
        friend Vector3<T>& operator*(Vector3<T>& left, const Vector3<T>& right) noexcept;
        friend Vector3<T>& operator*(Vector3<T>& left, const T& right) noexcept;
        friend Vector3<T>& operator/(Vector3<T>& left, const Vector3<T>& right) noexcept;
        friend Vector3<T>& operator/(Vector3<T>& left, const T& right) noexcept;
        friend Vector3<T>& operator%(Vector3<T>& left, const Vector3<T>& right) noexcept;
        friend Vector3<T>& operator%(Vector3<T>& left, const T& right) noexcept;
        friend std::ostream& operator<<(std::ostream& os, const Vector3<T>& right) noexcept;

        auto operator<=>(const Vector3<T>& right) const = default;

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

        inline Vector3<T>& operator%=(Vector3<T>& right) noexcept
        {
            x %= right.x;
            y %= right.y;
            z %= right.z;

            return *this;
        }

        inline Vector3<T>& operator%=(T& right) noexcept
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
    Vector3<T>& operator+(Vector3<T>& left, const Vector3<T>& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator+(Vector3<T>& left, const T& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator-(Vector3<T>& left, const Vector3<T>& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator-(Vector3<T>& left, const T& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator*(Vector3<T>& left, const Vector3<T>& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator*(Vector3<T>& left, const T& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator/(Vector3<T>& left, const Vector3<T>& right) noexcept
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator/(Vector3<T>& left, const T& right) noexcept
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator%(Vector3<T>& left, const Vector3<T>& right) noexcept
    {
        left %= right;
        return left;
    }
    template<typename T>
    Vector3<T>& operator%(Vector3<T>& left, const T& right) noexcept
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