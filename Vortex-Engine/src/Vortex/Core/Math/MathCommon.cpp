//
// Created by vitriol1744 on 07.07.2021.
//
#include "MathCommon.hpp"

#include <cmath>

namespace Vortex::Math
{
    float64 Sin(float64 number)
    {
        return sinf(number);
    }

    float64 Cos(float64 number)
    {
        return cosf(number);
    }

    float64 Tan(float64 number)
    {
        return tanf(number);
    }

    float64 ToRadians(float64 degrees)
    {
        return (degrees * (PI / 180));
    }

    float64 Sqrt(float64 number)
    {
        return sqrt(number);
    }
}