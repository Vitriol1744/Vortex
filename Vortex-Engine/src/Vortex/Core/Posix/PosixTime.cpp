//
// Created by Vitriol1744 on 29.06.2021.
//
#include "Core/Time.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_LINUX
namespace Vortex
{
    Time::Time()
    {

    }

    float64 Time::GetTimeInSeconds()
    {
        return 0.0;
    }
}
#endif