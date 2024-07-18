/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Log/Log.hpp"

namespace Vortex::Log
{
    static Logger s_CoreLogger   = "Core";
    static Logger s_ClientLogger = "Client";

    Logger&       GetCoreLogger() { return s_CoreLogger; }
    Logger&       GetClientLogger() { return s_ClientLogger; }
}; // namespace Vortex::Log
