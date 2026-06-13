/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/Version.hpp>
#include <Prism/Debug/SourceLocation.hpp>

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Export.hpp>
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Core/Platform.hpp>
#include <Vortex/Core/Types.hpp>

#define VtUnused(x) ((void)(x))
#define VtTodo()                                                               \
    {                                                                          \
        SourceLocation source = SourceLocation::Current();                     \
        VtUnused(source);                                                      \
        VtCoreWarnOnce("{} is not implemented", source.FunctionName());        \
    }
#define VtCoreBugOn(expr, ...)                                                 \
    if (!(expr)) {}                                                            \
    else {                                                                     \
        VtCoreWarn(__VA_ARGS__);                                               \
    }

namespace Vortex
{
#if defined(VT_COMPILER_CLANG) || defined(VT_COMPILER_GCC)
    #include <cxxabi.h>
    inline std::string Demangle(const char* type)
    {
        i32                                    status = -4;

        std::unique_ptr<char, void (*)(void*)> typeName(
            abi::__cxa_demangle(type, nullptr, nullptr, &status), std::free);

        return status == 0 ? typeName.get() : type;
    }

    #define VtGetTypeName(type) (Demangle(typeid(type).name()))
#else
    #define VtGetTypeName(type) (typeid(type).name())
#endif

    inline constexpr StringView g_EngineName = "Vortex Engine"_sv;
    inline constexpr Version    g_Version    = {0, 1, 0};
}; // namespace Vortex
