/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Assertions.hpp"
#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Log/Log.hpp"
#include "Vortex/Core/NonCopyable.hpp"
#include "Vortex/Core/Platform.hpp"
#include "Vortex/Core/Types.hpp"

#define VtUnused(x) ((void)(x))
#define VtTodo()                                                               \
    {                                                                          \
        std::source_location source = std::source_location::current();         \
        VtUnused(source);                                                     \
        VtCoreWarnOnce("{} is not implemented", source.function_name());       \
    }

namespace Vortex
{
    inline constexpr std::string_view g_EngineName = "Vortex Engine";
    inline constexpr Version          g_Version    = {0, 1, 0};
}; // namespace Vortex
