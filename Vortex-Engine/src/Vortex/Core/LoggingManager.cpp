//
// Created by vitriol1744 on 04.07.2021.
//
#include "LoggingManager.hpp"

#if defined(VT_PLATFORM_WINDOWS)
    #include "Vortex/Core/Win32/Win32Logger.hpp"
#elif defined(VT_PLATFORM_LINUX)
    #include "Vortex/Core/Posix/PosixLogger.hpp"
#endif

namespace Vortex
{
    LoggingManager::LoggingManager()
    {
        coreLogger  = CreateRef<LoggerImpl>("Vortex");
        logger      = CreateRef<LoggerImpl>("Client");
    }
}
