//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_ASSERTIONS_HPP
#define VORTEX_ASSERTIONS_HPP

#include "Vortex/Core/Logger.hpp"
#include "Vortex/Core/Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #define VT_DEBUGBREAK DebugBreak()
#elif defined(VT_PLATFORM_LINUX)
    #include <signal.h>
    #define VT_DEBUGBREAK raise(SIGINT)
#else
    #if defined(__arm__)
        #define VT_DEBUGBREAK __breakpoint(42)
    #elif defined(__x86_64__)
        #define VT_DEBUGBREAK asm("int $03")
    #elif defined(__thumb__)
        #define VT_DEBUGBREAK asm(".inst 0xde01")
    #elif defined(__aarch64__)
        #define VT_DEBUGBREAK asm(".inst 0xd4200000")
    #endif
#endif

#ifdef VT_ENABLE_ASSERTIONS
    #define VT_CORE_ASSERT(expr) VT_CORE_ASSERT_MSG(expr, #expr)
    #define VT_CORE_ASSERT_MSG(expr, msg) \
        if (expr) { } \
        else          \
        {             \
                                          \
            VTCoreLogFatal("Assertion Failed: {0}, In File: {1}, At Line: {2}", msg, __FILE__, __LINE__); \
            VT_DEBUG_BREAK;\
        }
    #define VT_ASSERT(expr) VT_ASSERT_MSG(expr, #expr)
    #define VT_ASSERT_MSG(expr, msg)\
        if (expr) { }\
        else \
        { \
                                    \
            VTCoreLogFatal("Assertion Failed: {0}, In File: {1}, At Line: {2}", msg, __FILE__, __LINE__);\
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
#endif //VORTEX_ASSERTIONS_HPP
