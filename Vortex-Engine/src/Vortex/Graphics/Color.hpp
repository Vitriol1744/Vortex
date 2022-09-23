//
// Created by Vitriol1744 on 19.09.2022.
//
#ifndef VORTEX_COLOR_HPP
#define VORTEX_COLOR_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    struct Color
    {
        inline constexpr Color(float32 r, float32 g, float32 b, float32 a) : r(r), g(g), b(b), a(a) { }
        inline constexpr Color(uint32 color)
            : r(static_cast<uint8>((color & 0xff000000) >> 24)), g(static_cast<uint8>((color & 0x00ff0000) >> 16)),
              b(static_cast<uint8>((color & 0x0000ff00) >> 8)), a(static_cast<uint8>((color & 0x000000ff) >> 00)) { }

        float32 r;
        float32 g;
        float32 b;
        float32 a;

        static const Color White;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Magenta;
        static const Color Cyan;
    };

    VT_NODISCARD inline constexpr const bool   operator ==(const Color& left, const Color& right);
    VT_NODISCARD inline constexpr const bool   operator !=(const Color& left, const Color& right);
    VT_NODISCARD inline const Color  operator  +(const Color& left, const Color& right);
    VT_NODISCARD inline const Color  operator  -(const Color& left, const Color& right);
    VT_NODISCARD inline const Color  operator  *(const Color& left, const Color& right);
    VT_NODISCARD inline const Color& operator +=(Color& left, const Color& right);
    VT_NODISCARD inline const Color& operator -=(Color& left, const Color& right);
    VT_NODISCARD inline const Color& operator *=(Color& left, const Color& right);

    #include "Vortex/Graphics/Color.inl"
}

#endif //VORTEX_COLOR_HPP
