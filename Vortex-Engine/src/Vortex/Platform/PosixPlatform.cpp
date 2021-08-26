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
        usleep(Timestep(seconds).Microseconds());
    }

    void Print(const char* text, size_t bytes)
    {
        ::write(STDOUT_FILENO, text, bytes);
    }
    void SetConsoleTextColor(ConsoleTextColor foregroundColor, ConsoleTextColor backgroundColor)
    {
        Print("\u001b[0m", sizeof("\u001b[0m") - 1);
        switch (foregroundColor)
        {
            case ConsoleTextColor::Default:                                                          break;
            case ConsoleTextColor::White:   Print("\u001b[37m", sizeof("\u001b[37m") - 1); break;
            case ConsoleTextColor::Black:   Print("\u001b[30m", sizeof("\u001b[30m") - 1); break;
            case ConsoleTextColor::Green:   Print("\u001b[32m", sizeof("\u001b[32m") - 1); break;
            case ConsoleTextColor::Cyan:    Print("\u001b[36m", sizeof("\u001b[36m") - 1); break;
            case ConsoleTextColor::Yellow:  Print("\u001b[33m", sizeof("\u001b[33m") - 1); break;
            case ConsoleTextColor::Red:     Print("\u001b[31m", sizeof("\u001b[31m") - 1); break;
        }
        switch (backgroundColor)
        {
            case ConsoleTextColor::Default:                                                          break;
            case ConsoleTextColor::White:   Print("\u001b[47m", sizeof("\u001b[47m") - 1); break;
            case ConsoleTextColor::Black:   Print("\u001b[40m", sizeof("\u001b[40m") - 1); break;
            case ConsoleTextColor::Green:   Print("\u001b[42m", sizeof("\u001b[42m") - 1); break;
            case ConsoleTextColor::Cyan:    Print("\u001b[46m", sizeof("\u001b[46m") - 1); break;
            case ConsoleTextColor::Yellow:  Print("\u001b[43m", sizeof("\u001b[43m") - 1); break;
            case ConsoleTextColor::Red:     Print("\u001b[41m", sizeof("\u001b[41m") - 1); break;
        }
    }
}
#endif