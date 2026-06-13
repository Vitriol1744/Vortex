/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Asset/AssetRegistry.hpp>
#include <Vortex/Asset/ShaderLibrary.hpp>

namespace Vortex
{
    namespace AssetManager
    {
        void Initialize();
        void Shutdown();

        PM_NODISCARD
        class AssetRegistry& Registry();
        PM_NODISCARD
        class ShaderLibrary& ShaderLibrary();
    }; // namespace AssetManager
}; // namespace Vortex
