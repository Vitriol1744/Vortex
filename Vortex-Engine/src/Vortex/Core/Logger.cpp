//
// Created by Vitriol1744 on 24.08.2021.
//
#include "Logger.hpp"

#include "Vortex/Platform/Platform.hpp"

namespace Vortex
{
    using namespace Platform;

    void Logger::Log(LogLevel level, std::string_view formattedString)
    {
        switch (level)
        {
            case LogLevel::eTrace:
                SetConsoleTextColor(ConsoleTextColor::eGreen, ConsoleTextColor::eDefault);
                break;
            case LogLevel::eInfo:
                SetConsoleTextColor(ConsoleTextColor::eCyan, ConsoleTextColor::eDefault);
                break;
            case LogLevel::eWarn:
                SetConsoleTextColor(ConsoleTextColor::eYellow, ConsoleTextColor::eDefault);
                break;
            case LogLevel::eError:
                SetConsoleTextColor(ConsoleTextColor::eRed, ConsoleTextColor::eDefault);
                break;
            case LogLevel::eFatal:
                SetConsoleTextColor(ConsoleTextColor::eBlack, ConsoleTextColor::eRed);
                break;

            default:
                SetConsoleTextColor(ConsoleTextColor::eWhite, ConsoleTextColor::eDefault);
                break;
        }

        Print("[", 1);
        Print(name.c_str(), name.size());
        Print("]: ", 3);
        Print(formattedString.data(), formattedString.size());

        SetConsoleTextColor(ConsoleTextColor::eWhite, ConsoleTextColor::eDefault);
        Print("\n", 1);
    }
    void Logger::Endl()
    {
        Print("\n", 1);
    }
}