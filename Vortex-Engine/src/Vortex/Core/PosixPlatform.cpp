//
// Created by Vitriol1744 on 11.08.2021.
//
#include "Platform.hpp"

#include "Vortex/Core/LoggingManager.hpp"
#include "Vortex/Core/Timestep.hpp"

#ifdef VT_PLATFORM_LINUX
#include <unistd.h>

namespace Vortex::Platform
{
    bool Internal::Initialize()
    {
		return true;
    }

    bool Internal::Shutdown()
    {
        return true;
    }

    float64 GetCurrentTime()
    {
        timespec currentTime{};
        clock_gettime(CLOCK_MONOTONIC, &currentTime);

        uint64 time = static_cast<uint64>(currentTime.tv_sec) * 1000000 + currentTime.tv_nsec / 1000;

        return static_cast<float64>(time / 1000000.0);
    }
    void Sleep(float64 seconds)
    {
        if (seconds <= 0) return;
        //usleep(Timestep(seconds).Microseconds());
    }
}
#endif