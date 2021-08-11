//
// Created by vitriol1744 on 04.07.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Singleton.hpp"
#include "Vortex/Core/Types.hpp"
#include "Vortex/Core/Platform.hpp"

#include "Vortex/Core/Logger.hpp"

#include "fmt/format.h"

namespace Vortex
{
    class VT_API LoggingManager : public Singleton<LoggingManager>
    {
        public:
            LoggingManager();
            
            [[nodiscard]] inline Ref<Logger> CoreLogger() const { return coreLogger; }
            [[nodiscard]] inline Ref<Logger> ClientLogger() const { return logger; }
            
        private:
            Ref<Logger> coreLogger;
            Ref<Logger> logger;
    };
}

#if defined(VT_DEBUG)
    #define VTCoreLogTrace(...)     LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Trace, fmt::format(__VA_ARGS__))
    #define VTCoreLogInfo(...)      LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Info, fmt::format(__VA_ARGS__))
    #define VTCoreLogWarn(...)      LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Warn, fmt::format(__VA_ARGS__))
    #define VTCoreLogError(...)     LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Error, fmt::format(__VA_ARGS__))
    #define VTCoreLogFatal(...)     LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Fatal, fmt::format(__VA_ARGS__))
    
    #define VTLogTrace(...)  Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Trace, fmt::format(__VA_ARGS__))
    #define VTLogInfo(...)   Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Info, fmt::format(__VA_ARGS__))
    #define VTLogWarn(...)   Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Warn, fmt::format(__VA_ARGS__))
    #define VTLogError(...)  Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Error, fmt::format(__VA_ARGS__))
    #define VTLogFatal(...)  Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Fatal, fmt::format(__VA_ARGS__))
#elif defined(VT_RELEASE)
    #define VTCoreLogTrace(fmt, ...)
    #define VTCoreLogInfo(fmt, ...)
    #define VTCoreLogWarn(fmt, ...)
    #define VTCoreLogError(fmt, ...)
    #define VTCoreLogFatal(fmt, ...)

    #define VTLogTrace(fmt, ...)
    #define VTLogInfo(fmt, ...)
    #define VTLogWarn(fmt, ...)
    #define VTLogError(fmt, ...)
    #define VTLogFatal(fmt, ...)
#endif
