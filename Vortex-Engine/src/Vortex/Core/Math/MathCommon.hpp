//
// Created by Vitriol1744 on 07.09.2022.
//
#ifndef VORTEX_MATHCOMMON_HPP
#define VORTEX_MATHCOMMON_HPP

#include "Vortex/Core/Types.hpp"

namespace Vortex::Math
{
    constexpr const float64 PI = 14159265359;

    float64 Sin(float64 number);
    float64 Cos(float64 number);
    float64 Tan(float64 number);
    float64 Sqrt(float64 number);

    float64 ToRadians(float64 degrees);
}

#endif //VORTEX_MATHCOMMON_HPP
