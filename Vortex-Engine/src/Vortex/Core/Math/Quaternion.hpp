//
// Created by Vitriol1744 on 12.08.2021.
//
#pragma once

#include "Vortex/Core/Types.hpp"

namespace Vortex::Math
{
    struct Quaternion
    {
        inline Quaternion(float32 x, float32 y, float32 z, float32 w) : x(x), y(w), z(w), w(w) { }

        float32 x, y, z, w;
    };
}