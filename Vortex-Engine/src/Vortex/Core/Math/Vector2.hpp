//
// Created by vitriol1744 on 16.07.2021.
//
#pragma once

#include "Core/Export.hpp"
#include "Core/Types.hpp"

#include <ostream>

namespace Vortex::Math
{
    template<typename T>
    struct VT_API Vector2
    {
        inline Vector2() noexcept = default;
        inline Vector2(T scalar) noexcept : x(scalar), y(scalar) { }
        inline Vector2(T x, T y) noexcept : x(x), y(y) { }

        union
        {
            T v[2];
            struct
            {
                T x, y;
            };

            struct
            {
                T r, g;
            };
        };

        inline T Dot(Vector2<T> right) const noexcept
        {
            return x * right.x + y * right.y;
        }

        // Operators
        friend Vector2<T>& operator+(Vector2<T>& left, const Vector2<T>& right) noexcept;
        friend Vector2<T>& operator+(Vector2<T>& left, const T& right) noexcept;
        friend Vector2<T>& operator-(Vector2<T>& left, const Vector2<T>& right) noexcept;
        friend Vector2<T>& operator-(Vector2<T>& left, const T& right) noexcept;
        friend Vector2<T>& operator*(Vector2<T>& left, const Vector2<T>& right) noexcept;
        friend Vector2<T>& operator*(Vector2<T>& left, const T& right) noexcept;
        friend Vector2<T>& operator/(Vector2<T>& left, const Vector2<T>& right) noexcept;
        friend Vector2<T>& operator/(Vector2<T>& left, const T& right) noexcept;
        friend Vector2<T>& operator%(Vector2<T>& left, const Vector2<T>& right) noexcept;
        friend Vector2<T>& operator%(Vector2<T>& left, const T& right) noexcept;
        friend std::ostream& operator<<(std::ostream& os, const Vector2<T> right) noexcept;

        auto operator<=>(const Vector2<T>& right) const = default;

        inline Vector2<T>& operator+=(Vector2<T>& right) noexcept
        {
            x += right.x;
            y += right.y;

            return *this;
        }

        inline Vector2<T>& operator+=(T& right) noexcept
        {
            x += right;
            y += right;

            return *this;
        }

        inline Vector2<T>& operator-=(Vector2<T>& right) noexcept
        {
            x -= right.x;
            y -= right.y;

            return *this;
        }

        inline Vector2<T>& operator-=(T& right) noexcept
        {
            x -= right;
            y -= right;

            return *this;
        }

        inline Vector2<T>& operator*=(Vector2<T>& right) noexcept
        {
            x *= right.x;
            y *= right.y;

            return *this;
        }

        inline Vector2<T>& operator*=(T& right) noexcept
        {
            x *= right;
            y *= right;

            return *this;
        }

        inline Vector2<T>& operator/=(Vector2<T>& right) noexcept
        {
            x /= right.x;
            y /= right.y;

            return *this;
        }

        inline Vector2<T>& operator/=(T& right) noexcept
        {
            x /= right;
            y /= right;

            return *this;
        }

        inline Vector2<T>& operator%=(Vector2<T>& right) noexcept
        {
            x %= right.x;
            y %= right.y;

            return *this;
        }

        inline Vector2<T>& operator%=(T& right) noexcept
        {
            x %= right;
            y %= right;

            return *this;
        }

        inline Vector2<T>& operator++() noexcept
        {
            x++;
            y++;

            return *this;
        }

        inline Vector2<T>& operator++(int) noexcept
        {
            Vector2<T> temp(*this);
            operator++();

            return temp;
        }

        inline Vector2<T>& operator--() noexcept
        {
            x--;
            y--;

            return *this;
        }

        inline Vector2<T>& operator--(int) noexcept
        {
            Vector2<T> temp(*this);
            operator--();

            return temp;
        }
    };

    template<typename T>
    Vector2<T>& operator+(Vector2<T>& left, const Vector2<T>& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator+(Vector2<T>& left, const T& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator-(Vector2<T>& left, const Vector2<T>& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator-(Vector2<T>& left, const T& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator*(Vector2<T>& left, const Vector2<T>& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator*(Vector2<T>& left, const T& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator/(Vector2<T>& left, const Vector2<T>& right) noexcept
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator/(Vector2<T>& left, const T& right) noexcept
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator%(Vector2<T>& left, const Vector2<T>& right) noexcept
    {
        left %= right;
        return left;
    }
    template<typename T>
    Vector2<T>& operator%(Vector2<T>& left, const T& right) noexcept
    {
        left %= right;
        return left;
    }
    template<typename T>
    std::ostream& operator<<(std::ostream& os, Vector2<T>& right) noexcept
    {
        os << "x: " << right.x << " y: " << right.y;
        os << '\n';

        return os;
    }


    using Vec2 = Vector2<float32>;
    using Vec2f = Vector2<float32>;
}