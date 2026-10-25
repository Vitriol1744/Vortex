/*
 * Created by v1tr10l7 on 26.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Export.hpp>

#define VT_ENGINE_API VT_API

namespace Vortex::Engine
{
    void VT_ENGINE_API InitializeCore();
    void VT_ENGINE_API ShutdownCore();
}; // namespace Engine
