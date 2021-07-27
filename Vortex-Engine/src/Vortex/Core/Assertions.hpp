//
// Created by Vitriol1744 on 25.06.2021.
//
#pragma once

#include "Vortex/Core/Macros.hpp"
#include "Vortex/Core/Logger.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #define VT_DEBUG_BREAK DebugBreak();
#elif defined(VT_PLATFORM_LINUX)
    #include <signal.h>
    #define VT_DEBUG_BREAK raise(SIGINT)
#else
    #ifdef _MSC_VER
        #define VT_DEBUG_BREAK __debugbreak()
    #elif defined(__arm__)
    #define VT_DEBUG_BREAK __breakpoint(42)
    #elif defined(__x86_64__)
        #define VT_DEBUG_BREAK asm("int $03")
    #elif defined(__thumb__	)
        #define VT_DEBUG_BREAK asm(".inst 0xde01")
    #elif defined(__aarch64__)
        #define VT_DEBUG_BREAK asm(".inst 0xd4200000")
    #endif
#endif

#ifdef VT_ENABLE_ASSERTIONS
    #define VT_CORE_ASSERT(expr) VT_CORE_ASSERT_MSG(expr, #expr)
    #define VT_CORE_ASSERT_MSG(expr, msg) \
        if (expr) { } \
        else \
        { \
            VT_CORE_LOG_FATAL("Assertion Failed: {0}, In File: {1}, At Line: {2}", msg, __FILE__, __LINE__); \
            VT_DEBUG_BREAK;\
        }
    #define VT_ASSERT(expr) VT_ASSERT_MSG(expr, #expr)
    #define VT_ASSERT_MSG(expr, msg)\
        if (expr) { }\
        else \
        { \
            VT_LOG_FATAL("Assertion Failed: {0}, In File: {1}, At Line: {2}", msg, __FILE__, __LINE__);\
            VT_DEBUG_BREAK;\
        }
    #ifdef DEBUG
        #define VT_CORE_SLOW_ASSERT(expr) VT_CORE_ASSERT(expr)
        #define VT_SLOW_ASSERT(expr) VT_ASSERT(expr)
    #else
        #define VT_CORE_SLOW_ASSERT(expr)
        #define VT_SLOW_ASSERT(expr)
    #endif
#else
    #define VT_CORE_ASSERT(expr, msg) expr
    #define VT_ASSERT(expr, msg) expr
    #define VT_CORE_SLOW_ASSERT(expr, msg) VT_CORE_ASSERT(expr, msg)
    #define VT_SLOW_ASSERT(expr, msg) VT_ASSERT(expr, msg)
#endif