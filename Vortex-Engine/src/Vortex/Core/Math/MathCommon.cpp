//
// Created by vitriol1744 on 07.07.2021.
//
#include "MathCommon.hpp"

#include <cmath>

namespace Vortex::Math
{
    double Tan(float64 number)
    {
        return tan(number);
    }

    double ToRadians(float64 degrees)
    {
        return (degrees * (PI / 180));
    }
}