//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

#include "Macros.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #ifdef VT_BUILD_DLL
        #define VT_API _declspec(dllexport)
    #else
        #define VT_API _declspec(dllimport)
    #endif
#elif defined(VT_PLATFORM_LINUX)
    #ifdef VT_BUILD_DLL
        #define VT_API __attribute__((dllexport))
    #else
        #define VT_API __attribute__((dllimport))
    #endif
#endif
