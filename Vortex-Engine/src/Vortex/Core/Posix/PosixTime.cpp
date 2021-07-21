//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Time.hpp"
#include "Core/Macros.hpp"

#ifdef VT_PLATFORM_LINUX
#include "unistd.h"

namespace Vortex
{
    Time::Time()
    {
        VT_CORE_LOG_INFO("Time Initialized!");
    }

    Timestep Time::GetTime()
    {
        timespec currentTime{};
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
    
        uint64 time = static_cast<uint64>(currentTime.tv_sec) * 1000000 + currentTime.tv_nsec / 1000;

        return Timestep((float64)time / 1000000.0f);
    }

    void Time::Sleep(Timestep timestep)
    {
        usleep(timestep.Microseconds());
    }
}
#endif