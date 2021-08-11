//
// Created by Vitriol1744 on 11.08.2021.
//
#include "Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include <Windows.h>

namespace Vortex::Platform
{
    namespace Internal
    {
        bool Internal::Initialize()
        {
            HRESULT hr;
            hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

            if (hr != S_OK)
            {
                if (hr == S_FALSE)
                {
                    VTCoreLogWarn("Platform Already Initialized!");
                }
                else
                {
                    VTCoreLogFatal("Failed to Initialize Platform!");
                    return false;
                }
            }

            return true;
        }

        bool Internal::Shutdown()
        {
            CoUninitialize();

            return true;
        }
    }

    static LARGE_INTEGER GetPerformanceFrequency()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        return frequency;
    }

    float64 Time::GetTime()
    {
        LARGE_INTEGER performanceCounter;
        static LARGE_INTEGER performanceFrequency = GetPerformanceFrequency();

        QueryPerformanceCounter(&performanceCounter);

        return (double)performanceCounter.QuadPart / (double)performanceFrequency.QuadPart;
    }

    void Time::Sleep(float64 seconds)
    {
        ::Sleep((unsigned long)Timestep(seconds).Milliseconds());
    }
}
#endif
