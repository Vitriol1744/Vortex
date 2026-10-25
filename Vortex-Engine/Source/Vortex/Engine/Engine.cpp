/*
 * Created by v1tr10l7 on 26.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Engine/Engine.hpp>

namespace Vortex::Engine
{
    void VT_ENGINE_API InitializeCore() { Log::Initialize(); }
    void VT_ENGINE_API ShutdownCore() { Log::Shutdown(); }
}; // namespace Vortex::Engine
