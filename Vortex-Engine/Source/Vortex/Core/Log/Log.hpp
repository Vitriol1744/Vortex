/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Debug/Logger.hpp>
#include <Vortex/Core/Export.hpp>

#include <mutex>

namespace Vortex::Log
{
    using Prism::Logger;
    using Prism::LogLevel;

    struct MutexPolicy
    {
        std::mutex Mutex;

        void       Lock() { Mutex.lock(); }
        void       Unlock() { Mutex.unlock(); }
    };

    void VT_CORE_API                 Initialize();
    void VT_CORE_API                 Shutdown();

    Logger<MutexPolicy>& VT_CORE_API CoreLogger();
    Logger<MutexPolicy>& VT_CORE_API ClientLogger();

    template <typename... Args>
    inline void Debug(fmt::format_string<Args...> format, Args&&... args);
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
    inline void DebugTag(StringView tag, fmt::format_string<Args...> format,
                         Args&&... args);
    template <typename... Args>
    inline void InfoTag(StringView tag, fmt::format_string<Args...> format,
                        Args&&... args);
    template <typename... Args>
    inline void WarnTag(StringView tag, fmt::format_string<Args...> format,
                        Args&&... args);
    template <typename... Args>
    inline void ErrorTag(StringView tag, fmt::format_string<Args...> format,
                         Args&&... args);

    template <typename... Args>
    void Debug(fmt::format_string<Args...> format, Args&&... args)
    {
        ClientLogger().Debug(format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void Trace(fmt::format_string<Args...> format, Args&&... args)
    {
        ClientLogger().Trace(format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void Info(fmt::format_string<Args...> format, Args&&... args)
    {
        ClientLogger().Info(format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void Warn(fmt::format_string<Args...> format, Args&&... args)
    {
        ClientLogger().Warn(format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void Error(fmt::format_string<Args...> format, Args&&... args)
    {
        ClientLogger().Error(format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void Fatal(fmt::format_string<Args...> format, Args&&... args)
    {
        ClientLogger().Fatal(format, Forward<Args>(args)...);
    }

    template <typename... Args>
    void DebugTag(StringView tag, fmt::format_string<Args...> format,
                  Args&&... args)
    {
        ClientLogger().DebugTag(tag, format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void TraceTag(StringView tag, fmt::format_string<Args...> format,
                  Args&&... args)
    {
        ClientLogger().TraceTag(tag, format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void InfoTag(StringView tag, fmt::format_string<Args...> format,
                 Args&&... args)
    {
        ClientLogger().InfoTag(tag, format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void WarnTag(StringView tag, fmt::format_string<Args...> format,
                 Args&&... args)
    {
        ClientLogger().WarnTag(tag, format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void ErrorTag(StringView tag, fmt::format_string<Args...> format,
                  Args&&... args)
    {
        ClientLogger().ErrorTag(tag, format, Forward<Args>(args)...);
    }
    template <typename... Args>
    void FatalTag(StringView tag, fmt::format_string<Args...> format,
                  Args&&... args)
    {
        ClientLogger().FatalTag(tag, format, Forward<Args>(args)...);
    }
}; // namespace Vortex::Log

#ifndef VT_DIST
    #define VtCoreDebug(...) Vortex::Log::CoreLogger().Debug(__VA_ARGS__)
    #define VtCoreTrace(...) Vortex::Log::CoreLogger().Trace(__VA_ARGS__)
    #define VtCoreInfo(...)  Vortex::Log::CoreLogger().Info(__VA_ARGS__)
    #define VtCoreWarn(...)  Vortex::Log::CoreLogger().Warn(__VA_ARGS__)
    #define VtCoreError(...) Vortex::Log::CoreLogger().Error(__VA_ARGS__)
    #define VtCoreFatal(...) Vortex::Log::CoreLogger().Fatal(__VA_ARGS__)

    #define VtCoreWarnOnce(...)                                                \
        do                                                                     \
        {                                                                      \
            static AtomicBool warned = false;                                  \
            if (!warned.Exchange(true, MemoryOrder::eRelaxed))                 \
            {                                                                  \
                VtCoreWarn(__VA_ARGS__);                                       \
            }                                                                  \
            warned = true;                                                     \
        } while (false)

    #define VtDebug(...) Vortex::Log::Debug(__VA_ARGS__);
    #define VtTrace(...) Vortex::Log::Trace(__VA_ARGS__);
    #define VtInfo(...)  Vortex::Log::Info(__VA_ARGS__);
    #define VtWarn(...)  Vortex::Log::Warn(__VA_ARGS__);
    #define VtError(...) Vortex::Log::Error(__VA_ARGS__);
    #define VtFatal(...) Vortex::Log::Fatal(__VA_ARGS__);

    #define VtCoreDebugTag(tag, ...)                                            \
        Vortex::Log::CoreLogger().DebugTag(tag, __VA_ARGS__)
    #define VtCoreTraceTag(tag, ...)                                            \
        Vortex::Log::CoreLogger().TraceTag(tag, __VA_ARGS__)
    #define VtCoreInfoTag(tag, ...)                                            \
        Vortex::Log::CoreLogger().InfoTag(tag, __VA_ARGS__)
    #define VtCoreWarnTag(tag, ...)                                            \
        Vortex::Log::CoreLogger().WarnTag(tag, __VA_ARGS__)
    #define VtCoreErrorTag(tag, ...)                                           \
        Vortex::Log::CoreLogger().ErrorTag(tag, __VA_ARGS__)
    #define VtCoreFatalTag(tag, ...)                                            \
        Vortex::Log::CoreLogger().FatalTag(tag, __VA_ARGS__)

    #define VtDebugTag(tag, ...)  Vortex::Log::DebugTag(tag, __VA_ARGS__)
    #define VtTraceTag(tag, ...)  Vortex::Log::TraceTag(tag, __VA_ARGS__)
    #define VtInfoTag(tag, ...)  Vortex::Log::InfoTag(tag, __VA_ARGS__)
    #define VtWarnTag(tag, ...)  Vortex::Log::WarnTag(tag, __VA_ARGS__)
    #define VtErrorTag(tag, ...) Vortex::Log::ErrorTag(tag, __VA_ARGS__)
    #define VtFatalTag(tag, ...)  Vortex::Log::FatalTag(tag, __VA_ARGS__)
#else
    #define VtCoreDebug(...)
    #define VtCoreTrace(...)
    #define VtCoreInfo(...)
    #define VtCoreWarn(...)
    #define VtCoreError(...)
    #define VtCoreFatal(...)
    #define VtCoreWarnOnce(...)

    #define VtDebug(...)
    #define VtTrace(...)
    #define VtInfo(...)
    #define VtWarn(...)
    #define VtError(...)
    #define VtFatal(...)

    #define VtCoreDebugTag(...)
    #define VtCoreTraceTag(...)
    #define VtCoreInfoTag(...)
    #define VtCoreWarnTag(...)
    #define VtCoreErrorTag(...)
    #define VtCoreFatalTag(...)

    #define VtDebugTag(...)
    #define VtTraceTag(...)
    #define VtInfoTag(...)
    #define VtWarnTag(...)
    #define VtErrorTag(...)
    #define VtFatalTag(...)
#endif
