/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Platform.hpp"

#ifdef VT_COMPILER_MSVC
    #define VT_EXPORT _declspec(dllexport)
    #define VT_IMPORT _declspec(dllimport)
#else
    #define VT_EXPORT __attribute__((visibility("default")))
    #define VT_IMPORT __attribute__((visibility("default")))
#endif

#ifndef VT_STATIC_LIBRARY
    #ifdef VT_BUILD_DLL
        #define VT_API VT_EXPORT
    #else
        #define VT_API VT_IMPORT
    #endif
#else
    #define VT_API
#endif
