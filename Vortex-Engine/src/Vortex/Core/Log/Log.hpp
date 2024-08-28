/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Log/Logger.hpp"

namespace Vortex::Log
{
    Logger& GetCoreLogger();
    Logger& GetClientLogger();

    template <typename... Args>
    inline void Trace(fmt::format_string<Args...> format, Args&&... args);
    template <typename... Args>
    inline void Info(fmt::format_string<Args...> format, Args&&... args);
    template <typename... Args>
    inline void Warn(fmt::format_string<Args...> format, Args&&... args);
    template <typename... Args>
    inline void Error(fmt::format_string<Args...> format, Args&&... args);
    template <typename... Args>
    inline void Fatal(fmt::format_string<Args...> format, Args&&... args);

    template <typename... Args>
    void Trace(fmt::format_string<Args...> format, Args&&... args)
    {
        GetClientLogger().Trace(format, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Info(fmt::format_string<Args...> format, Args&&... args)
    {
        GetClientLogger().Info(format, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Warn(fmt::format_string<Args...> format, Args&&... args)
    {
        GetClientLogger().Warn(format, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Error(fmt::format_string<Args...> format, Args&&... args)
    {
        GetClientLogger().Error(format, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void Fatal(fmt::format_string<Args...> format, Args&&... args)
    {
        GetClientLogger().Fatal(format, std::forward<Args>(args)...);
    }
}; // namespace Vortex::Log

#ifndef VT_DIST
    #define VtCoreTrace(...) Log::GetCoreLogger().Trace(__VA_ARGS__);
    #define VtCoreInfo(...)  Log::GetCoreLogger().Info(__VA_ARGS__);
    #define VtCoreWarn(...)  Log::GetCoreLogger().Warn(__VA_ARGS__);
    #define VtCoreError(...) Log::GetCoreLogger().Error(__VA_ARGS__);
    #define VtCoreFatal(...) Log::GetCoreLogger().Fatal(__VA_ARGS__);

    #define VtCoreWarnOnce(...)                                                \
        {                                                                      \
            static bool warned = false;                                        \
            if (!warned) VtCoreWarn(__VA_ARGS__);                              \
            warned = true;                                                     \
        }

    #define VtTrace(...) Vortex::Log::Trace(__VA_ARGS__);
    #define VtInfo(...)  Vortex::Log::Info(__VA_ARGS__);
    #define VtWarn(...)  Vortex::Log::Warn(__VA_ARGS__);
    #define VtError(...) Vortex::Log::Error(__VA_ARGS__);
    #define VtFatal(...) Vortex::Log::Fatal(__VA_ARGS__);
#else
    #define VtCoreTrace(...)
    #define VtCoreInfo(...)
    #define VtCoreWarn(...)
    #define VtCoreError(...)
    #define VtCoreFatal(...)

    #define VtTrace(...)
    #define VtInfo(...)
    #define VtWarn(...)
    #define VtError(...)
    #define VtFatal(...)
#endif
