/*
 * Created by v1tr10l7 on 11.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Engine/RuntimeAssetManager.hpp>

namespace Vortex
{
    ::Ref<class Asset> RuntimeAssetManager::Asset(const AssetHandle& handle)
    {
        return nullptr;
    }
    ::Ref<TextureAsset> RuntimeAssetManager::LoadTexture2D(PathView path)
    {
        return CreateRef<TextureAsset>(path);
    }
}; // namespace Vortex
