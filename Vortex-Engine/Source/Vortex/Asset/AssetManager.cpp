/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Asset/AssetManager.hpp>

namespace Vortex
{
    namespace AssetManager
    {
        namespace
        {
            AssetRegistry       s_Registry;
            class ShaderLibrary s_ShaderLibrary;
        }; // namespace

        void Initialize() {}
        void Shutdown()
        {
            s_Registry.Clear();
            s_ShaderLibrary.Clear();
        }

        PM_NODISCARD
        AssetRegistry& Registry() { return s_Registry; }
        PM_NODISCARD
        class ShaderLibrary& ShaderLibrary() { return s_ShaderLibrary; }
    }; // namespace AssetManager
}; // namespace Vortex
