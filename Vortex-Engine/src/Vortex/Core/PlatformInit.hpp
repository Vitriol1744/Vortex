//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#if defined(_WIN64)
    #define VT_PLATFORM_WINDOWS
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #error Vortex Currently Supports only x64 builds of Windows!
#elif defined(__ANDROID__)
    #error Vortex Currently Doesn't Support Windows!
#elif defined(__linux__)
    #define VT_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #error Vortex Currently Doesn't Support IOS Simulator!
    #elif TARGET_OS_MACCATALYST
        #error Vortex Currently Doesn't Support Mac's Catalyst!
    #elif TARGET_OS_IPHONE
        #error Vortex Currently Doesn't Support IOS!
    #elif TARGET_OS_MAC
        #error Vortex Currently Doesn't Support Mac!
    #else
        #error Unknown Apple Platform!
    #endif
#elif __unix__
    #error Unknown Unix System!
#else
    #error Unknown Platform!
#endif

#ifdef DEBUG
    #define VT_DEBUG
#else
    #define VT_RELEASE
#endif

namespace Vortex::Platform
{
    bool Initialize();
    bool Shutdown();
}