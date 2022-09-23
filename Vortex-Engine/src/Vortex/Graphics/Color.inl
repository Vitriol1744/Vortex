//
// Created by Vitriol1744 on 19.09.2022.
//
#ifndef VORTEX_COLOR_INL
#define VORTEX_COLOR_INL

inline constexpr Color Color::White      = { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Color Color::Black      = { 0.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::Red        = { 1.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Color Color::Green      = { 0.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::Blue       = { 0.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::Magenta    = { 1.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Color Color::Yellow     = { 1.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Color Color::Cyan       = { 0.0f, 1.0f, 1.0f, 1.0f };

constexpr const bool operator==(const Color& lhs, const Color& rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b) && (lhs.a == rhs.a);
}
constexpr const bool operator!=(const Color& lhs, const Color& rhs)
{
    return !(lhs == rhs);
}
const Color operator+(const Color& lhs, const Color& rhs)
{
    float32 r = std::clamp(lhs.r + rhs.r, 0.0f, 1.0f);
    float32 g = std::clamp(lhs.g + rhs.g, 0.0f, 1.0f);
    float32 b = std::clamp(lhs.b + rhs.b, 0.0f, 1.0f);
    float32 a = std::clamp(lhs.a + rhs.a, 0.0f, 1.0f);

    return Color(r, g, b, a);
}
const Color operator-(const Color& lhs, const Color& rhs)
{
    float32 r = std::clamp(lhs.r - rhs.r, 0.0f, 1.0f);
    float32 g = std::clamp(lhs.g - rhs.g, 0.0f, 1.0f);
    float32 b = std::clamp(lhs.b - rhs.b, 0.0f, 1.0f);
    float32 a = std::clamp(lhs.a - rhs.a, 0.0f, 1.0f);

    return Color(r, g, b, a);
}
const Color operator*(const Color& lhs, const Color& rhs)
{
    float32 r = std::clamp(lhs.r * rhs.r, 0.0f, 1.0f);
    float32 g = std::clamp(lhs.g * rhs.g, 0.0f, 1.0f);
    float32 b = std::clamp(lhs.b * rhs.b, 0.0f, 1.0f);
    float32 a = std::clamp(lhs.a * rhs.a, 0.0f, 1.0f);

    return Color(r, g, b, a);
}
const Color& operator+=(Color& left, const Color& right)
{
    return left = left + right;
}
const Color& operator-=(Color& left, const Color& right)
{
    return left = left - right;
}
const Color& operator*=(Color& left, const Color& right)
{
    return left = left * right;
}

#endif //VORTEX_COLOR_INL
