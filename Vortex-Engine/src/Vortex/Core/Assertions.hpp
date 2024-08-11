/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Log/Log.hpp"
#include "Vortex/Core/Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #define VtDebugBreak DebugBreak()
#elif defined(VT_COMPILER_CLANG) || defined(VT_COMPILER_GCC)
    #define VtDebugBreak __builtin_trap()
#elif defined(VT_PLATFORM_LINUX)
    #include <signal.h>
    #define VtDebugBreak raise(SIGINT)
#elif defined(__arm__)
    #define VtDebugBreak __breakpoint(42)
#elif defined(__x86_64__)
    #define VtDebugBreak asm("int $03")
#elif defined(__thumb__)
    #define VtDebugBreak asm(".inst 0xde01")
#elif defined(__aarch64__)
    #define VtDebugBreak asm(".inst 0xd4200000")
#else
    #warning "VtDebugBreak not implemented on this platform"
#endif

#ifdef VT_ENABLE_ASSERTIONS
    #define VtPrintCoreAssertMessage(expr, msg)                                \
        {                                                                      \
            std::source_location source = std::source_location::current();     \
            (void)source;                                                      \
            VtCoreFatal("Assertion Failed: '{}' ->\n{}[{}:{}] ->\n{}", msg,    \
                        source.file_name(), source.line(), source.column(),    \
                        source.function_name());                               \
        }

    #define VtCoreAssert(expr) VtCoreAssertMsg(expr, #expr)
    #define VtCoreAssertMsg(expr, msg)                                         \
        if (expr) {}                                                           \
        else                                                                   \
        {                                                                      \
            VtPrintCoreAssertMessage(expr, msg);                               \
            VtDebugBreak;                                                      \
        }
    #define VtCoreAssertFormat(expr, ...)                                      \
        if (expr) {}                                                           \
        else                                                                   \
        {                                                                      \
            std::string formatted = fmt::format(__VA_ARGS__);                  \
            VtPrintCoreAssertMessage(expr, formatted);                         \
            VtDebugBreak;                                                      \
        }
    #define VtPrintAssertMessage(expr, msg)                                    \
        {                                                                      \
            std::source_location source = std::source_location::current();     \
            (void)source;                                                      \
            VtFatal("Assertion Failed: '{}' ->\n{}[{}:{}] ->\n{}", msg,        \
                    source.file_name(), source.line(), source.column(),        \
                    source.function_name());                                   \
            VtDebugBreak;                                                      \
        }
    #define VtAssert(expr) VtAssertMsg(expr, #expr)
    #define VtAssertMsg(expr, msg)                                             \
        if (expr) {}                                                           \
        else                                                                   \
        {                                                                      \
            VtPrintAssertMessage(expr, msg);                                   \
            VtDebugBreak;                                                      \
        }
    #define VtAssertFormat(expr, ...)                                          \
        if (expr) {}                                                           \
        else                                                                   \
        {                                                                      \
            auto formatted = fmt::format(__VA_ARGS__);                         \
            VtPrintAssertMessage(expr, formatted);                             \
            VtDebugBreak;                                                      \
        }

    #ifdef VT_DEBUG
        #define VtCoreSlowAssert(expr) VtCoreAssert(expr)
        #define VtSlowAssert(expr)     VtAssert(expr)
    #else
        #define VtCoreSlowAssert(expr)
        #define VtSlowAssert(expr)
    #endif
#else
    #define VtCoreAssert(expr, msg)     expr
    #define VtAssert(expr, msg)         expr
    #define VtCoreSlowAssert(expr, msg) VtCoreAssert(expr, msg)
    #define VtSlowAssert(expr, msg)     VtAssert(expr, msg)
#endif
