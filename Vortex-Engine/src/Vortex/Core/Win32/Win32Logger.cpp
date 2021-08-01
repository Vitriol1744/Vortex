//
// Created by Vitriol1744 on 24.06.2021.
//
#include "vtpch.hpp"
#include "Vortex/Core/PlatformInit.hpp"

#ifdef VT_PLATFORM_WINDOWS
#include "Win32Logger.hpp"
#include <windows.h>
namespace Vortex
{
    void Logger::Initialize()
    {

    }

    void LoggerImpl::Log(LogLevel level, std::string_view formattedString)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        switch (level)
        {
            case LogLevel::Trace:
                SetConsoleTextAttribute(hConsole, 0x02);
                break;
            case LogLevel::Info:
                SetConsoleTextAttribute(hConsole, 0x09);
                break;
            case LogLevel::Warn:
                SetConsoleTextAttribute(hConsole, 0x06);
                break;
            case LogLevel::Error:
                SetConsoleTextAttribute(hConsole, 0x04);
                break;
            case LogLevel::Fatal:
                SetConsoleTextAttribute(hConsole,  0x40);
                break;
        }

        WriteConsoleA(hConsole, "[", 1, nullptr, nullptr);
        WriteConsoleA(hConsole, name.data(), name.size(), nullptr, nullptr);
        WriteConsoleA(hConsole, "]: ", 3, nullptr, nullptr);
        WriteConsoleA(hConsole, formattedString.data(), formattedString.size(), nullptr, nullptr);
        SetConsoleTextAttribute(hConsole, 0x07);

        WriteConsoleA(hConsole, "\r\n", 2, nullptr, nullptr);
    }
}
#endif
