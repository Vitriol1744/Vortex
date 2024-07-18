/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Log/Log.hpp"
#include "Vortex/Core/Log/Logger.hpp"

namespace Vortex
{
    namespace
    {
        inline constexpr const char* COLOR_DEFAULT           = "\u001b[0m";
        inline constexpr const char* FOREGROUND_COLOR_RED    = "\u001b[31m";
        inline constexpr const char* FOREGROUND_COLOR_GREEN  = "\u001b[32m";
        inline constexpr const char* FOREGROUND_COLOR_YELLOW = "\u001b[33m";
        inline constexpr const char* FOREGROUND_COLOR_BLUE   = "\u001b[34m";
        inline constexpr const char* FOREGROUND_COLOR_CYAN   = "\u001b[36m";
        inline constexpr const char* FOREGROUND_COLOR_WHITE  = "\u001b[37m";

        inline constexpr const char* BACKGROUND_COLOR_RED    = "\u001b[41m";
    }; // namespace

    void Logger::Log(LogLevel level, std::string_view message)
    {
        if (level < m_LogLevelFilter) return;
        auto levelString = magic_enum::enum_name(level);
        printf("%s[%s", COLOR_DEFAULT,
               this == &Log::GetCoreLogger() ? FOREGROUND_COLOR_BLUE
                                             : COLOR_DEFAULT);
        printf("%s%s::", m_Name.data(), COLOR_DEFAULT);

        switch (level)
        {
            case LogLevel::eTrace: printf("%s", FOREGROUND_COLOR_GREEN); break;
            case LogLevel::eInfo: printf("%s", FOREGROUND_COLOR_CYAN); break;
            case LogLevel::eWarn: printf("%s", FOREGROUND_COLOR_YELLOW); break;
            case LogLevel::eError: printf("%s", FOREGROUND_COLOR_RED); break;
            case LogLevel::eFatal:
                printf("%s%s", FOREGROUND_COLOR_WHITE, BACKGROUND_COLOR_RED);
                break;

            default: break;
        }

        printf("%s%s]: ", levelString.data() + 1, COLOR_DEFAULT);
        printf("%s\n", message.data());
    };
}; // namespace Vortex
