//
// Created by Vitriol1744 on 05.09.2022.
//
#include "Logger.hpp"

namespace Vortex
{
    void Logger::Log(LogLevel log_level, std::string_view formatted_message)
    {
        switch (log_level)
        {
            case LogLevel::eTrace:
                break;
            case LogLevel::eInfo:
                break;
            case LogLevel::eWarn:
                break;
            case LogLevel::eError:
                break;
            case LogLevel::eFatal:
                break;
        }
    }
}