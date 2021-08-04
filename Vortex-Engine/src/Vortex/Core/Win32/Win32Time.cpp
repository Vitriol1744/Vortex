//
// Created by Vitriol1744 on 28.06.2021.
//
#include "vtpch.hpp"
#include "Vortex/Core/Time.hpp"
#include "Vortex/Core/PlatformInit.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>

namespace Vortex
{
    namespace
    {
        LARGE_INTEGER GetPerformanceFrequency()
        {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency(&frequency);

            return frequency;
        }
    }

    Timestep Time::GetTime()
    {
        LARGE_INTEGER performanceCounter;
        static LARGE_INTEGER performanceFrequency = GetPerformanceFrequency();

        QueryPerformanceCounter(&performanceCounter);

        return Timestep((double)performanceCounter.QuadPart / (double)performanceFrequency.QuadPart);
    }

    void Time::Sleep(Timestep timestep)
    {
        ::Sleep((unsigned long)timestep.Milliseconds());
    }
}
#endif