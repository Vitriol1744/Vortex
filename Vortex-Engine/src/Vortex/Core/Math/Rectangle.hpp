//
// Created by Vitriol1744 on 10.08.2021.
//
#pragma once

namespace Vortex::Math
{
    struct VT_API Rectangle
    {
        inline Rectangle() : x(0), y(0), width(0), height(0) { }
        inline Rectangle(int32 x, int32 y, int32 width, int32 height) : x(x), y(y), width(width), height(height) { }

        int32 x, y;
        int32 width, height;
    };
}
