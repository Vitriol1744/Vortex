//
// Created by Vitriol1744 on 07.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "MathCommon.hpp"

namespace Vortex::Math
{
    float64 Sin(float64 number)
    {
        return glm::sin(number);
    }
    float64 Cos(float64 number)
    {
        return glm::cos(number);
    }
    float64 Tan(float64 number)
    {
        return glm::tan(number);
    }
    float64 Sqrt(float64 number)
    {
        return glm::sqrt(number);
    }

    float64 ToRadians(float64 degrees)
    {
        return glm::radians(degrees);
    }
}