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
    static HANDLE hConsole;

    static LARGE_INTEGER GetPerformanceFrequency()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);

        return frequency;
    }

    float64 GetTime()
    {
        LARGE_INTEGER performanceCounter;
        static LARGE_INTEGER performanceFrequency = GetPerformanceFrequency();

        QueryPerformanceCounter(&performanceCounter);

        return (double)performanceCounter.QuadPart / (double)performanceFrequency.QuadPart;
    }

    void Sleep(float64 seconds)
    {
        ::Sleep((unsigned long)Timestep(seconds).Milliseconds());
    }

    void Print(const char* text, size_t bytes)
    {
        if (!hConsole) hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WriteConsoleA(hConsole, text, bytes, nullptr, nullptr);
    }
    void SetConsoleTextColor(ConsoleTextColor foregroundColor, ConsoleTextColor backgroundColor)
    {
        if (!hConsole) hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int background = 0;
        int foreground = 0;
        switch (foregroundColor)
        {
            case ConsoleTextColor::Default:                    break;
            case ConsoleTextColor::White:   foreground = 0x0F; break;
            case ConsoleTextColor::Black:   foreground = 0x00; break;
            case ConsoleTextColor::Green:   foreground = 0x02; break;
            case ConsoleTextColor::Cyan:    foreground = 0x09; break;
            case ConsoleTextColor::Yellow:  foreground = 0x06; break;
            case ConsoleTextColor::Red:     foreground = 0x04; break;
        }
        switch (backgroundColor)
        {
            case ConsoleTextColor::Default:                    break;
            case ConsoleTextColor::White:   background = 0xF0; break;
            case ConsoleTextColor::Black:   background = 0x00; break;
            case ConsoleTextColor::Green:   background = 0x20; break;
            case ConsoleTextColor::Cyan:    background = 0x90; break;
            case ConsoleTextColor::Yellow:  background = 0x60; break;
            case ConsoleTextColor::Red:     background = 0x40; break;
        }
        SetConsoleTextAttribute(hConsole, background & foreground);
    }
}
#endif
