//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#ifdef WIN32
    #define VT_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define VT_PLATFORM_LINUX
#else
    #error Platform Not Supported!
#endif

#ifdef DEBUG
    #define VT_DEBUG
#else
    #define VT_RELEASE
#endif
