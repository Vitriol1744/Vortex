/*
 * Created by v1tr10l7 on 22.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once
#pragma once

import Prism.Debug.LogSink;

#ifdef VT_PLATFORM_ANDROID
    #include <android/log.h>

namespace Vortex::Log
{
    template <typename LockingPolicy>
    class AndroidSink final : public LogSink<LockingPolicy>
    {
      public:
        explicit AndroidSink(StringView tag = "VortexEngine")
            : m_Tag(tag)
        {
        }

        isize SinkIt(const LogMessage& message) override
        {
            android_LogPriority priority = MapPriority(message.Level);

            // Format stack buffer or null-terminate safely
            String              payloadStr(message.Payload);
            String              tagStr(message.LoggerName.Empty()
                                           ? m_Tag.Raw()
                                           : message.LoggerName.Raw());

            IgnoreUnused(__android_log_print(priority, tagStr.Raw(), "%s",
                                             payloadStr.Raw()));
            return message.Payload.Size();
        }

        isize WriteNoLock(StringView str) override
        {
            String payloadStr(str);
            String tagStr(m_Tag);
            IgnoreUnused(__android_log_print(ANDROID_LOG_INFO, tagStr.Raw(),
                                             "%s", payloadStr.Raw()));
            return str.Size();
        }

      private:
        static android_LogPriority MapPriority(LogLevel level)
        {
            switch (level)
            {
                case LogLevel::eTrace: return ANDROID_LOG_VERBOSE;
                case LogLevel::eDebug: return ANDROID_LOG_DEBUG;
                case LogLevel::eInfo: return ANDROID_LOG_INFO;
                case LogLevel::eWarn: return ANDROID_LOG_WARN;
                case LogLevel::eError: return ANDROID_LOG_ERROR;
                case LogLevel::eFatal: return ANDROID_LOG_FATAL;
                default: return ANDROID_LOG_DEFAULT;
            }
        }

        String m_Tag;
    };
} // namespace Vortex::Log
#endif
