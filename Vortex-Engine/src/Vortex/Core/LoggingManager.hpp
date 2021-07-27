//
// Created by vitriol1744 on 04.07.2021.
//
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Singleton.hpp"
#include "Vortex/Core/Types.hpp"
#include "Vortex/Core/Macros.hpp"

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
    #define VT_CORE_LOG_TRACE(...)     LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Trace, fmt::format(__VA_ARGS__))
    #define VT_CORE_LOG_INFO(...)      LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Info, fmt::format(__VA_ARGS__))
    #define VT_CORE_LOG_WARN(...)      LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Warn, fmt::format(__VA_ARGS__))
    #define VT_CORE_LOG_ERROR(...)     LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Error, fmt::format(__VA_ARGS__))
    #define VT_CORE_LOG_FATAL(...)     LoggingManager::Instance()->CoreLogger()->Log(LogLevel::Fatal, fmt::format(__VA_ARGS__))
    
    #define VT_LOG_TRACE(...)  Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Trace, fmt::format(__VA_ARGS__))
    #define VT_LOG_INFO(...)   Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Info, fmt::format(__VA_ARGS__))
    #define VT_LOG_WARN(...)   Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Warn, fmt::format(__VA_ARGS__))
    #define VT_LOG_ERROR(...)  Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Error, fmt::format(__VA_ARGS__))
    #define VT_LOG_FATAL(...)  Vortex::LoggingManager::Instance()->ClientLogger()->Log(Vortex::LogLevel::Fatal, fmt::format(__VA_ARGS__))
#elif defined(VT_RELEASE)
    #define VT_CORE_LOG_TRACE(fmt, ...)
    #define VT_CORE_LOG_INFO(fmt, ...)
    #define VT_CORE_LOG_WARN(fmt, ...)
    #define VT_CORE_LOG_ERROR(fmt, ...)
    #define VT_CORE_LOG_FATAL(fmt, ...)

    #define VT_LOG_TRACE(fmt, ...)
    #define VT_LOG_INFO(fmt, ...)
    #define VT_LOG_WARN(fmt, ...)
    #define VT_LOG_ERROR(fmt, ...)
    #define VT_LOG_FATAL(fmt, ...)
#endif
