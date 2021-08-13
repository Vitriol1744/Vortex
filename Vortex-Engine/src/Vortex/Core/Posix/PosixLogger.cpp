//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Vortex/Core/Platform.hpp"
#include "Vortex/Core/Platform.hpp"

#ifdef VT_PLATFORM_LINUX
#include "PosixLogger.hpp"
#include <unistd.h>

namespace Vortex
{
    using namespace Platform;
    void Logger::Initialize()
    {

    }
    
    void LoggerImpl::Log(LogLevel level, std::string_view formattedString)
    {
        switch (level)
        {
            case LogLevel::Trace:
                SetConsoleTextColor(ConsoleTextColor::Green, ConsoleTextColor::Default);
                break;
            case LogLevel::Info:
                SetConsoleTextColor(ConsoleTextColor::Cyan, ConsoleTextColor::Default);
                break;
            case LogLevel::Warn:
                SetConsoleTextColor(ConsoleTextColor::Yellow, ConsoleTextColor::Default);
                break;
            case LogLevel::Error:
                SetConsoleTextColor(ConsoleTextColor::Red, ConsoleTextColor::Default);
                break;
            case LogLevel::Fatal:
                SetConsoleTextColor(ConsoleTextColor::Black, ConsoleTextColor::Red);
                break;
                
            default:
                SetConsoleTextColor(ConsoleTextColor::White, ConsoleTextColor::Default);
                break;
        }
        
        Print("[", 1);
        Print(name.c_str(), name.size());
        Print("]: ", 3);
        Print(formattedString.data(), formattedString.size());
        
        SetConsoleTextColor(ConsoleTextColor::White, ConsoleTextColor::Default);
        Print("\n", 1);
    }
}
#endif