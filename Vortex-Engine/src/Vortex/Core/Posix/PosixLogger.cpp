//
// Created by Vitriol1744 on 29.06.2021.
//
#include "vtpch.hpp"
#include "Core/Macros.hpp"

#include <iostream>

#ifdef VT_PLATFORM_LINUX
#include "PosixLogger.hpp"
#include "unistd.h"

namespace Vortex
{
    void Logger::Initialize()
    {
        //TODO: Initialize Logger
    }
    
    void LoggerImpl::Log(LogLevel level, std::string_view formattedString)
    {
        // Don't print null terminator character
        ::write(STDOUT_FILENO, "\u001b[0m", sizeof("\u001b[0m") - 1);
        switch (level)
        {
            case LogLevel::Trace:
                ::write(STDOUT_FILENO, "\u001b[32m", sizeof("\u001b[32m") - 1);
                break;
            case LogLevel::Info:
                ::write(STDOUT_FILENO, "\u001b[36m", sizeof("\u001b[36m") - 1);
                break;
            case LogLevel::Warn:
                ::write(STDOUT_FILENO, "\u001b[33m", sizeof("\u001b[33m") - 1);
                break;
            case LogLevel::Error:
                ::write(STDOUT_FILENO, "\u001b[31m", sizeof("\u001b[31m") - 1);
                break;
            case LogLevel::Fatal:
                ::write(STDOUT_FILENO, "\u001b[37m\u001b[41m", sizeof("\u001b[37m\u001b[41m") - 1);
                break;
                
            default:
                ::write(STDOUT_FILENO, "\u001b[37m", sizeof("\u001b[37m") - 1);
                ::write(STDOUT_FILENO, "\u001b[40m", sizeof("\u001b[40") - 1);
                break;
        }
        
        ::write(STDOUT_FILENO, name.c_str(), name.size());
        ::write(STDOUT_FILENO, formattedString.data(), formattedString.size());
        
        // Change color before printing new line character
        ::write(STDOUT_FILENO, "\u001b[0m", sizeof("\u001b[0m") - 1);
        ::write(STDOUT_FILENO, "\n", 1);
    }
}
#endif