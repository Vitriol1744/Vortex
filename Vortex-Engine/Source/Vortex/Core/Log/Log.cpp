/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
import Prism.Containers.Array;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
#include <Vortex/Core/Log/Log.hpp>

#ifdef VT_PLATFORM_ANDROID
    #include <Vortex/Core/Log/Sinks/AndroidSink.hpp>
#endif

namespace Vortex::Log
{
    static Logger<MutexPolicy> s_CoreLogger("Core");
    static Logger<MutexPolicy> s_ClientLogger("Client");
    /// The sinks Initialize created. Loggers keep raw pointers, and other
    /// code (the editor's console) registers sinks it owns itself, so only
    /// these are released here.
    static Vector<::Ref<LogSinkBase>> s_OwnedSinks;

    void VT_CORE_API Initialize()
    {
        auto loggers
            = ToArray<Logger<MutexPolicy>*>({&s_CoreLogger, &s_ClientLogger});
        for (auto logger : loggers)
        {
#ifdef VT_PLATFORM_ANDROID
            ::Ref<LogSinkBase> sink = CreateRef<AndroidSink<MutexPolicy>>();
#else
            ::Ref<LogSinkBase> sink = CreateRef<StdOutSink>();
#endif
            logger->AddSink(sink.Raw());
            s_OwnedSinks.PushBack(sink);
        }
    }
    void VT_CORE_API Shutdown()
    {
        s_CoreLogger.RemoveAllSinks();
        s_ClientLogger.RemoveAllSinks();
        s_OwnedSinks.Clear();
    }

    Logger<MutexPolicy>& VT_CORE_API CoreLogger() { return s_CoreLogger; }
    Logger<MutexPolicy>& VT_CORE_API ClientLogger() { return s_ClientLogger; }
}; // namespace Vortex::Log
