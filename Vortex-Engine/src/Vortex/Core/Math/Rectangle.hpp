//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_RECTANGLE_HPP
#define VORTEX_RECTANGLE_HPP

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex::Math
{
    struct VT_API Rectangle
    {
        inline Rectangle() : x(0), y(0), width(0), height(0) { }
        inline Rectangle(int32 x, int32 y, int32 width, int32 height)
            : x(x), y(y), width(width), height(height) { }

        int32 x, y;
        int32 width, height;
    };
}

#endif //VORTEX_RECTANGLE_HPP
