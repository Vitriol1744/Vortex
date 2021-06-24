//
// Created by Vitriol1744 on 22.06.2021.
//
#pragma once

#include "Export.hpp"
#include "Macros.hpp"
#include "Types.hpp"
#include "Singleton.hpp"
#include "Logger.hpp"

#ifdef VT_ENABLE_ASSERTIONS
    #define VT_CORE_ASSERT(expr, msg) \
        if (expr); \
        else \
        { \
            VT_CORE_LOG_FATAL("Assertion Failed: {0}, In File: {1}, At Line: {2}", msg, __FILE__, __LINE__); \
                                     \
        }
    #define VT_ASSERT(expr, msg)\
        if (expr);\
        else \
        { \
        VT_LOG_FATAL("Assertion Failed: {0}, In File: {1}, At Line: {2}", msg, __FILE__, __LINE__);\
                               \
        }
    #ifdef DEBUG
        #define VT_CORE_SLOW_ASSERT(expr, msg) VT_CORE_ASSERT(expr, msg)
        #define VT_SLOW_ASSERT(expr, msg) VT_ASSERT(expr, msg)
    #endif
#else
    #define VT_CORE_ASSERT(expr, msg) expr
    #define VT_ASSERT(expr, msg) expr
    #define VT_CORE_SLOW_ASSERT(expr, msg) VT_CORE_ASSERT(expr, msg)
    #define VT_SLOW_ASSERT(expr, msg) VT_ASSERT(expr, msg)
#endif