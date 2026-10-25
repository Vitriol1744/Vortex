/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>
#include <Vortex/Core/Log/Log.hpp>

#ifdef VT_PLATFORM_ANDROID
    #include <Vortex/Core/Log/Sinks/AndroidSink.hpp>
#endif

namespace Vortex::Log
{
    Prism::StdOutSink          s_DummySink;

    static Logger<MutexPolicy> s_CoreLogger("Core");
    static Logger<MutexPolicy> s_ClientLogger("Client");

    void VT_CORE_API           Initialize()
    {
        auto loggers
            = ToArray<Logger<MutexPolicy>*>({&s_CoreLogger, &s_ClientLogger});

        for (auto logger : loggers)
#ifdef VT_PLATFORM_ANDROID
            logger->AddSink(new AndroidSink<MutexPolicy>);
#else
            logger->AddSink(new StdOutSink);
#endif
    }
    void VT_CORE_API Shutdown()
    {
        auto loggers
            = ToArray<Logger<MutexPolicy>*>({&s_CoreLogger, &s_ClientLogger});
        for (auto logger : loggers)
        {
            logger->ForEachSink(
                [](LogSinkBase* sink) -> IterationResult
                {
                    delete sink;
                    return IterationResult::eContinue;
                });
            logger->RemoveAllSinks();
        }
    }

    Logger<MutexPolicy>& VT_CORE_API CoreLogger() { return s_CoreLogger; }
    Logger<MutexPolicy>& VT_CORE_API ClientLogger() { return s_ClientLogger; }
}; // namespace Vortex::Log
