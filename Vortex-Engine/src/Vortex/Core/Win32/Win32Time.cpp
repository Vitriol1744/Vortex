//
// Created by Vitriol1744 on 28.06.2021.
//
#include "vtpch.hpp"
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

    Timestep Time::GetTimeInSeconds()
    {
        QueryPerformanceCounter(&counter);

        return Timestep((double)counter.QuadPart / (double)frequency.QuadPart);
    }

    void Time::Sleep(Timestep timestep)
    {
        Sleep(timestep.Milliseconds());
    }
}
#endif