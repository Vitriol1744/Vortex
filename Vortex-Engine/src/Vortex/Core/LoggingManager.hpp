//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_LOGGING_MANAGER_HPP
#define VORTEX_LOGGING_MANAGER_HPP

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Singleton.hpp"
#include "Vortex/Core/Types.hpp"

#include "Vortex/Core/Logger.hpp"

namespace Vortex
{
    class VT_API LoggingManager : public Singleton<LoggingManager>
    {
        public:
            LoggingManager()
            {
                core_logger = CreateRef<Logger>("Core");
                client_logger = CreateRef<Logger>("Client");
            }

            VT_NODISCARD inline Ref<Logger> GetCoreLogger() const noexcept { return core_logger; }
            VT_NODISCARD inline Ref<Logger> GetClientLogger() const noexcept { return client_logger; }

        private:
            Ref<Logger> core_logger;
            Ref<Logger> client_logger;
    };

    template class VT_API Singleton<LoggingManager>;
}

#if defined(VT_DEBUG)
    #define VTCoreLogTrace(...)     LoggingManager::Instance()->GetCoreLogger()->Log(LogLevel::eTrace, std::format(__VA_ARGS__))
    #define VTCoreLogInfo(...)      LoggingManager::Instance()->GetCoreLogger()->Log(LogLevel::eInfo, std::format(__VA_ARGS__))
    #define VTCoreLogWarn(...)      LoggingManager::Instance()->GetCoreLogger()->Log(LogLevel::eWarn, std::format(__VA_ARGS__))
    #define VTCoreLogError(...)     LoggingManager::Instance()->GetCoreLogger()->Log(LogLevel::eError, std::format(__VA_ARGS__))
    #define VTCoreLogFatal(...)     LoggingManager::Instance()->GetCoreLogger()->Log(LogLevel::eFatal, std::format(__VA_ARGS__))

    #define VTLogTrace(...)  Vortex::LoggingManager::Instance()->GetClientLogger()->Log(Vortex::LogLevel::eTrace, std::format(__VA_ARGS__))
    #define VTLogInfo(...)   Vortex::LoggingManager::Instance()->GetClientLogger()->Log(Vortex::LogLevel::eInfo, std::format(__VA_ARGS__))
    #define VTLogWarn(...)   Vortex::LoggingManager::Instance()->GetClientLogger()->Log(Vortex::LogLevel::eWarn, std::format(__VA_ARGS__))
    #define VTLogError(...)  Vortex::LoggingManager::Instance()->GetClientLogger()->Log(Vortex::LogLevel::eError, std::format(__VA_ARGS__))
    #define VTLogFatal(...)  Vortex::LoggingManager::Instance()->GetClientLogger()->Log(Vortex::LogLevel::eFatal, std::format(__VA_ARGS__))
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

#endif //VORTEX_LOGGING_MANAGER_HPP
