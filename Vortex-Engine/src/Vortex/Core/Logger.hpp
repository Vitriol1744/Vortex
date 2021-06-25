//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Export.hpp"
#include "Macros.hpp"

#include <Windows.h>

#include <xstring>
#include <format>
#include <iostream>

namespace Vortex
{
    enum class LogLevel
    {
        Trace,
        Info,
        Warn,
        Error,
        Fatal
    };

    class VT_API Logger
    {
        public:
            static void Initialize();

            template<typename... Args>
            inline static void Log(LogLevel level, std::string_view format, Args... args)
            {
                instance->LogImpl(level, pattern + std::format(format, std::forward<Args&>(args)...));
            }

            template<typename... Args>
            inline static void CoreLog(LogLevel level, std::string_view format, Args... args)
            {
                instance->LogImpl(level, corePattern + std::format(format, std::forward<Args&>(args)...));
            }

        protected:
            static Logger* instance;
            static std::string pattern;
            static std::string corePattern;

            virtual void InitializeImpl() = 0;
            virtual void LogImpl(LogLevel level, std::string_view formattedString) = 0;
    };
}

#ifdef DEBUG
    #define VT_CORE_LOG_TRACE(fmt, ...) Logger::CoreLog(LogLevel::Trace, fmt, __VA_ARGS__)
    #define VT_CORE_LOG_INFO(fmt, ...) Logger::CoreLog(LogLevel::Info, fmt, __VA_ARGS__)
    #define VT_CORE_LOG_WARN(fmt, ...) Logger::CoreLog(LogLevel::Warn, fmt, __VA_ARGS__)
    #define VT_CORE_LOG_ERROR(fmt, ...) Logger::CoreLog(LogLevel::Error, fmt, __VA_ARGS__)
    #define VT_CORE_LOG_FATAL(fmt, ...) Logger::CoreLog(LogLevel::Fatal, fmt, __VA_ARGS__)

    #define VT_LOG_TRACE(fmt, ...) Vortex::Logger::Log(Vortex::LogLevel::Trace, fmt, __VA_ARGS__)
    #define VT_LOG_INFO(fmt, ...) Vortex::Logger::Log(Vortex::LogLevel::Info, fmt, __VA_ARGS__)
    #define VT_LOG_WARN(fmt, ...) Vortex::Logger::Log(Vortex::LogLevel::Warn, fmt, __VA_ARGS__)
    #define VT_LOG_ERROR(fmt, ...) Vortex::Logger::Log(Vortex::LogLevel::Error, fmt, __VA_ARGS__)
    #define VT_LOG_FATAL(fmt, ...) Vortex::Logger::Log(Vortex::LogLevel::Fatal, fmt, __VA_ARGS__)
#else
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

