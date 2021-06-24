//
// Created by Vitriol1744 on 24.06.2021.
//
#include "Logger.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include "Win32/Win32Logger.hpp"
#else
    #error Platform Not Supported!
#endif

namespace Vortex
{
    Logger* Logger::instance = nullptr;
    std::string Logger::pattern;
    std::string Logger::corePattern;

    void Logger::Initialize()
    {
        if (!instance) instance = new LoggerImpl;

        pattern = "[Client]: ";
        corePattern = "[Vortex]: ";

        instance->InitializeImpl();
    }
}