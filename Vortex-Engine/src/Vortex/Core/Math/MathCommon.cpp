//
// Created by vitriol1744 on 07.07.2021.
//
#include "MathCommon.hpp"

#include <cmath>

namespace Vortex::Math
{
    double Tan(double number)
    {
        return tan(number);
    }

    double ToRadians(double degrees)
    {
        return (degrees * (pi / 180));
    }
}