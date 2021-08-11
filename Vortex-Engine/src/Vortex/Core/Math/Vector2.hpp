//
// Created by vitriol1744 on 16.07.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

#include <ostream>

namespace Vortex::Math
{
    template<typename T>
    struct Vector2
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
        template<typename T2>
        friend Vector2<T2> operator+(Vector2<T2> left, const Vector2<T2>& right) noexcept;
        template<typename T2>
        friend Vector2<T2> operator+(Vector2<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector2<T2> operator-(Vector2<T2> left, const Vector2<T2>& right) noexcept;
        template<typename T2>
        friend Vector2<T2> operator-(Vector2<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector2<T2> operator*(Vector2<T2> left, const Vector2<T2>& right) noexcept;
        template<typename T2>
        friend Vector2<T2> operator*(Vector2<T2> left, const T2& right) noexcept;
        template<typename T2>
        friend Vector2<T2> operator/(Vector2<T2> left, const Vector2<T2>& right);
        template<typename T2>
        friend Vector2<T2> operator/(Vector2<T2> left, const T2& right);
        template<typename T2>
        friend Vector2<T2> operator%(Vector2<T2> left, const Vector2<T2>& right);
        template<typename T2>
        friend Vector2<T2> operator%(Vector2<T2> left, const T2& right);
        template<typename T2>
        friend std::ostream& operator<<(std::ostream& os, const Vector2<T2>& right) noexcept;

        inline Vector2<T>& operator+=(const Vector2<T>& right) noexcept
        {
            x += right.x;
            y += right.y;

            return *this;
        }
        inline Vector2<T>& operator+=(Vector2<T>&& right) noexcept
        {
            x += right.x;
            y += right.y;

            return *this;
        }
        inline Vector2<T>& operator+=(const T& right) noexcept
        {
            x += right;
            y += right;

            return *this;
        }
        inline Vector2<T>& operator+=(T&& right) noexcept
        {
            x += right;
            y += right;

            return *this;
        }
        inline Vector2<T>& operator-=(const Vector2<T>& right) noexcept
        {
            x -= right.x;
            y -= right.y;

            return *this;
        }
        inline Vector2<T>& operator-=(Vector2<T>&& right) noexcept
        {
            x -= right.x;
            y -= right.y;

            return *this;
        }
        inline Vector2<T>& operator-=(const T& right) noexcept
        {
            x -= right;
            y -= right;

            return *this;
        }
        inline Vector2<T>& operator-=(T&& right) noexcept
        {
            x -= right;
            y -= right;

            return *this;
        }
        inline Vector2<T>& operator*=(const Vector2<T>& right) noexcept
        {
            x *= right.x;
            y *= right.y;

            return *this;
        }
        inline Vector2<T>& operator*=(Vector2<T>&& right) noexcept
        {
            x *= right.x;
            y *= right.y;

            return *this;
        }
        inline Vector2<T>& operator*=(const T& right) noexcept
        {
            x *= right;
            y *= right;

            return *this;
        }
        inline Vector2<T>& operator*=(T&& right) noexcept
        {
            x *= right;
            y *= right;

            return *this;
        }
        inline Vector2<T>& operator/=(const Vector2<T>& right)
        {
            x /= right.x;
            y /= right.y;

            return *this;
        }
        inline Vector2<T>& operator/=(Vector2<T>&& right)
        {
            x /= right.x;
            y /= right.y;

            return *this;
        }
        inline Vector2<T>& operator/=(const T& right)
        {
            x /= right;
            y /= right;

            return *this;
        }
        inline Vector2<T>& operator/=(T&& right)
        {
            x /= right;
            y /= right;

            return *this;
        }
        inline Vector2<T>& operator%=(const Vector2<T>& right) noexcept
        {
            x %= right.x;
            y %= right.y;

            return *this;
        }
        inline Vector2<T>& operator%=(Vector2<T>&& right) noexcept
        {
            x %= right.x;
            y %= right.y;

            return *this;
        }
        inline Vector2<T>& operator%=(const T& right) noexcept
        {
            x %= right;
            y %= right;

            return *this;
        }
        inline Vector2<T>& operator%=(T&& right) noexcept
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
    Vector2<T> operator+(Vector2<T> left, const Vector2<T>& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector2<T> operator+(Vector2<T> left, const T& right) noexcept
    {
        left += right;
        return left;
    }
    template<typename T>
    Vector2<T> operator-(Vector2<T> left, const Vector2<T>& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator-(Vector2<T> left, const T& right) noexcept
    {
        left -= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator*(Vector2<T> left, const Vector2<T>& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator*(Vector2<T> left, const T& right) noexcept
    {
        left *= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator/(Vector2<T> left, const Vector2<T>& right)
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator/(Vector2<T> left, const T& right)
    {
        left /= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator%(Vector2<T> left, const Vector2<T>& right)
    {
        left %= right;
        return left;
    }
    template<typename T>
    Vector2<T> operator%(Vector2<T> left, const T& right)
    {
        left %= right;
        return left;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Vector2<T>& right) noexcept
    {
        os << "x: " << right.x << " y: " << right.y;
        os << '\n';

        return os;
    }


    using Vec2  = Vector2<float32>;
    using Vec2f = Vector2<float32>;
    using Vec2u = Vector2<uint32>;
    using Vec2i = Vector2<int32>;
}