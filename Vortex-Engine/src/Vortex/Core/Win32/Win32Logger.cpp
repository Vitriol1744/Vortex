//
// Created by Vitriol1744 on 24.06.2021.
//
#include "Win32Logger.hpp"

namespace Vortex
{
    void LoggerImpl::InitializeImpl()
    {

    }

    void LoggerImpl::LogImpl(LogLevel level, std::string_view formattedString)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        switch (level)
        {
            case LogLevel::Trace:
                SetConsoleTextAttribute(hConsole, 11);
                break;
            case LogLevel::Info:
                SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
                break;
            case LogLevel::Warn:
                SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
                break;
            case LogLevel::Error:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                break;
            case LogLevel::Fatal:
                SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
                break;
        }

        WriteConsoleA(hConsole, formattedString.data(), formattedString.size(), nullptr, nullptr);
        WriteConsoleA(hConsole, "\r\n", 2, nullptr, nullptr);

        SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
}