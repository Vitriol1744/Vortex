//
// Created by vitriol1744 on 04.07.2021.
//
#pragma once

#include "Core/Export.hpp"
#include "Core/Singleton.hpp"
#include "Core/Types.hpp"
#include "Core/Macros.hpp"

#include "Core/Logger.hpp"

namespace Vortex
{
    class VT_API LoggingManager : public Singleton<LoggingManager>
    {
        public:
            LoggingManager();
            
            [[nodiscard]] inline Ref<Logger> GetCoreLogger() const { return coreLogger; }
            [[nodiscard]] inline Ref<Logger> GetLogger() const { return logger; }
            
        private:
            Ref<Logger> coreLogger;
            Ref<Logger> logger;
    };
}

#if defined(VT_DEBUG)
#define VT_CORE_LOG_TRACE(...)     LoggingManager::Get()->GetCoreLogger()->Log(LogLevel::Trace, fmt::format(__VA_ARGS__))
#define VT_CORE_LOG_INFO(...)      LoggingManager::Get()->GetCoreLogger()->Log(LogLevel::Info, fmt::format(__VA_ARGS__))
#define VT_CORE_LOG_WARN(...)      LoggingManager::Get()->GetCoreLogger()->Log(LogLevel::Warn, fmt::format(__VA_ARGS__))
#define VT_CORE_LOG_ERROR(...)     LoggingManager::Get()->GetCoreLogger()->Log(LogLevel::Error, fmt::format(__VA_ARGS__))
#define VT_CORE_LOG_FATAL(...)     LoggingManager::Get()->GetCoreLogger()->Log(LogLevel::Fatal, fmt::format(__VA_ARGS__))

#define VT_LOG_TRACE(...)  Vortex::LoggingManager::Get()->GetLogger()->Log(Vortex::LogLevel::Trace, fmt::format(__VA_ARGS__))
#define VT_LOG_INFO(...)   Vortex::LoggingManager::Get()->GetLogger()->Log(Vortex::LogLevel::Info, fmt::format(__VA_ARGS__))
#define VT_LOG_WARN(...)   Vortex::LoggingManager::Get()->GetLogger()->Log(Vortex::LogLevel::Warn, fmt::format(__VA_ARGS__))
#define VT_LOG_ERROR(...)  Vortex::LoggingManager::Get()->GetLogger()->Log(Vortex::LogLevel::Error, fmt::format(__VA_ARGS__))
#define VT_LOG_FATAL(...)  Vortex::LoggingManager::Get()->GetLogger()->Log(Vortex::LogLevel::Fatal, fmt::format(__VA_ARGS__))
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
