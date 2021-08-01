//
// Created by Vitriol1744 on 28.06.2021.
//
#include "vtpch.hpp"
#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/PlatformInit.hpp"

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

    Timestep Time::GetTime()
    {
        QueryPerformanceCounter(&counter);

        return Timestep((double)counter.QuadPart / (double)frequency.QuadPart);
    }

    void Time::Sleep(Timestep timestep)
    {
        ::Sleep((unsigned long)timestep.Milliseconds());
    }
}
#endif