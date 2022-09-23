//
// Created by Vitriol1744 on 14.09.2022.
//
#define VT_EXPORT_WIN32_HEADERS
#include "Vortex/vtpch.hpp"
#include "Platform.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #define INTERNAL_DEBUG_BREAK ::DebugBreak()
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

namespace Vortex
{
    void DebugBreak()
    {
        INTERNAL_DEBUG_BREAK;
    }
}