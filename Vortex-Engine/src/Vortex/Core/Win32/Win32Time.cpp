//
// Created by Vitriol1744 on 28.06.2021.
//
#include "Core/Time.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>

namespace
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
}

namespace Vortex
{
    Time::Time()
    {
        QueryPerformanceFrequency(&frequency);
    }

    float64 Time::GetTimeInSeconds()
    {
        QueryPerformanceCounter(&counter);

        return (double)counter.QuadPart / (double)frequency.QuadPart;
    }
}
#endif