/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    enum class LogLevel : u8
    {
        ePlain,
        eTrace,
        eInfo,
        eWarn,
        eError,
        eFatal,
    };

    class VT_API Logger
    {
      public:
        Logger() = default;
        inline Logger(const char* name)
            : m_Name(name)
        {
        }
        inline Logger(std::string_view name)
            : m_Name(name)
        {
        }

        inline std::string_view GetName() const { return m_Name; }
        inline LogLevel GetLogLevelFilter() const { return m_LogLevelFilter; }

        inline void     SetName(std::string_view name) { m_Name = name; }
        inline void     SetLogLevelFilter(LogLevel logLevelFilter)
        {
            m_LogLevelFilter = logLevelFilter;
        }

        void Log(LogLevel logLevel, std::string_view message);

        template <typename... Args>
        inline void Log(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::ePlain, format, std::forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Log(LogLevel logLevel, fmt::format_string<Args...> format,
                        Args&&... args)
        {
            Log(logLevel, fmt::format(format, std::forward<Args>(args)...));
        }

        template <typename... Args>
        inline void Trace(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eTrace, format, std::forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Info(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eInfo, format, std::forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Warn(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eWarn, format, std::forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Error(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eError, format, std::forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Fatal(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eFatal, format, std::forward<Args>(args)...);
        }

      private:
        std::string m_Name           = "Logger";
        LogLevel    m_LogLevelFilter = LogLevel::eTrace;
    };
}; // namespace Vortex
