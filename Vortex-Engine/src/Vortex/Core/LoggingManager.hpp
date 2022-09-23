//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_LOGGING_MANAGER_HPP
#define VORTEX_LOGGING_MANAGER_HPP

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Singleton.hpp"
#include "Vortex/Core/Types.hpp"
#include "Vortex/Core/Platform.hpp"

#include "Vortex/Core/Logger.hpp"

namespace Vortex
{
    class LoggingManager : public Singleton<LoggingManager>
    {
        public:
            LoggingManager()
            {
                coreLogger = Logger("Core");
                clientLogger = Logger("Client");
            }

            VT_NODISCARD inline Logger& GetCoreLogger() const noexcept { return coreLogger; }
            VT_NODISCARD inline Logger& GetClientLogger() const noexcept { return clientLogger; }

        private:
            inline static Logger coreLogger;
            inline static Logger clientLogger;
    };
}

#ifndef VT_DIST
    #define VTCoreLogTrace(...)     LoggingManager::GetInstance()->GetCoreLogger().Log(LogLevel::eTrace, fmt::format(__VA_ARGS__))
    #define VTCoreLogInfo(...)      LoggingManager::GetInstance()->GetCoreLogger().Log(LogLevel::eInfo, fmt::format(__VA_ARGS__))
    #define VTCoreLogWarn(...)      LoggingManager::GetInstance()->GetCoreLogger().Log(LogLevel::eWarn, fmt::format(__VA_ARGS__))
    #define VTCoreLogError(...)     LoggingManager::GetInstance()->GetCoreLogger().Log(LogLevel::eError, fmt::format(__VA_ARGS__))
    #define VTCoreLogFatal(...)     LoggingManager::GetInstance()->GetCoreLogger().Log(LogLevel::eFatal, fmt::format(__VA_ARGS__))

    #define VTLogTrace(...)  Vortex::LoggingManager::GetInstance()->GetClientLogger().Log(Vortex::LogLevel::eTrace, fmt::format(__VA_ARGS__))
    #define VTLogInfo(...)   Vortex::LoggingManager::GetInstance()->GetClientLogger().Log(Vortex::LogLevel::eInfo, fmt::format(__VA_ARGS__))
    #define VTLogWarn(...)   Vortex::LoggingManager::GetInstance()->GetClientLogger().Log(Vortex::LogLevel::eWarn, fmt::format(__VA_ARGS__))
    #define VTLogError(...)  Vortex::LoggingManager::GetInstance()->GetClientLogger().Log(Vortex::LogLevel::eError, fmt::format(__VA_ARGS__))
    #define VTLogFatal(...)  Vortex::LoggingManager::GetInstance()->GetClientLogger().Log(Vortex::LogLevel::eFatal, fmt::format(__VA_ARGS__))
#else
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

#endif //VORTEX_LOGGING_MANAGER_HPP
