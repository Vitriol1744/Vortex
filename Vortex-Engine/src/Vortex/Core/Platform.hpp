/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#if defined(_WIN64) || defined(WIN32)
    #define VT_PLATFORM_WINDOWS
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #error "Vortex Currently Supports only x64 builds of Windows!"
#elif defined(__ANDROID__)
    #error "Vortex Currently Doesn't Support Windows!"
#elif defined(__linux__)
    #define VT_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #error "Vortex Currently Doesn't Support IOS Simulator!"
    #elif TARGET_OS_MACCATALYST
        #error "Vortex Currently Doesn't Support Mac's Catalyst!"
    #elif TARGET_OS_IPHONE
        #error "Vortex Currently Doesn't Support IOS!"
    #elif TARGET_OS_MAC
        #error "Vortex Currently Doesn't Support Mac!"
    #else
        #error "Unknown Apple Platform!"
    #endif
#elif __unix__
    #error "Unknown Unix System!"
#else
    #error "Unknown Platform!"
#endif

#if defined(__clang__)
    #define VT_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define VT_COMPILER_GCC
#elif defined(_MSC_VER)
    #define VT_COMPILER_MSVC
#endif
