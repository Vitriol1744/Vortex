//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_ASSERTIONS_HPP
#define VORTEX_ASSERTIONS_HPP

#include "Vortex/Core/Logger.hpp"
#include "Vortex/Core/Platform.hpp"

#define VT_DEBUG_BREAK Vortex::DebugBreak()


#ifdef VT_ENABLE_ASSERTIONS
    #define VT_FILE_NAME std::source_location::current().file_name()
    #define VT_LINE std::source_location::current().line()
    #define VT_COLUMN std::source_location::current().column()
    #define VT_FUNC_SIG std::source_location::current().function_name()

    #define VT_CORE_ASSERT(expr) VT_CORE_ASSERT_MSG(expr, #expr)
    #define VT_CORE_ASSERT_MSG(expr, ...) \
        if (expr) { } \
        else          \
        {             \
                                          \
            VTCoreLogFatal("Assertion Failed: {}, \nfile: {}({}:{})\nfunction: {}", __VA_ARGS__, VT_FILE_NAME, VT_LINE, VT_COLUMN, VT_FUNC_SIG); \
            VT_DEBUG_BREAK;\
        }
    #define VT_ASSERT(expr) VT_ASSERT_MSG(expr, #expr)
    #define VT_ASSERT_MSG(expr, ...)\
        if (expr) { }\
        else \
        { \
                                    \
            VTLogFatal("Assertion Failed: {}, \nfile: {}({}:{})\nfunction: {}", __VA_ARGS__, VT_FILE_NAME, VT_LINE, VT_COLUMN, VT_FUNC_SIG); \
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
