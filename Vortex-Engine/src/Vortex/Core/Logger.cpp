//
// Created by Vitriol1744 on 05.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "Logger.hpp"

namespace Vortex
{
    enum class ConsoleTextColor
    {
        eDefault,
        eWhite,
        eBlack,
        eGreen,
        eCyan,
        eYellow,
        eRed,
    };

    static void SetConsoleTextColor(ConsoleTextColor foregroundColor, ConsoleTextColor backgroundColor)
    {
        std::cout << "\u001b[0m";
        switch (foregroundColor)
        {
            case ConsoleTextColor::eDefault:                            break;
            case ConsoleTextColor::eWhite:   std::cout << "\u001b[37m"; break;
            case ConsoleTextColor::eBlack:   std::cout << "\u001b[30m"; break;
            case ConsoleTextColor::eGreen:   std::cout << "\u001b[32m"; break;
            case ConsoleTextColor::eCyan:    std::cout << "\u001b[36m"; break;
            case ConsoleTextColor::eYellow:  std::cout << "\u001b[33m"; break;
            case ConsoleTextColor::eRed:     std::cout << "\u001b[31m"; break;
        }
        switch (backgroundColor)
        {
            case ConsoleTextColor::eDefault:                            break;
            case ConsoleTextColor::eWhite:   std::cout << "\u001b[47m"; break;
            case ConsoleTextColor::eBlack:   std::cout << "\u001b[40m"; break;
            case ConsoleTextColor::eGreen:   std::cout << "\u001b[42m"; break;
            case ConsoleTextColor::eCyan:    std::cout << "\u001b[46m"; break;
            case ConsoleTextColor::eYellow:  std::cout << "\u001b[43m"; break;
            case ConsoleTextColor::eRed:     std::cout << "\u001b[41m"; break;
        }
    }

    void Logger::Log(LogLevel log_level, std::string_view formatted_message)
    {
        switch (log_level)
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
                SetConsoleTextColor(ConsoleTextColor::eWhite, ConsoleTextColor::eRed);
                break;
        }

        std::cout << '[' << name << "]: " << formatted_message << '\n';
    }
}