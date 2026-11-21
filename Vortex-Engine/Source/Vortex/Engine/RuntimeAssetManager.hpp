/*
 * Created by v1tr10l7 on 11.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Asset/AssetManager.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>

namespace Vortex
{
    class RuntimeAssetManager : public AssetManager
    {
      public:
        RuntimeAssetManager()                   = default;
        virtual ~RuntimeAssetManager() override = default;

        virtual ::Ref<class Asset> Asset(const AssetHandle& handle) override;

        ::Ref<TextureAsset>        LoadTexture2D(PathView path);

        /// Loaded on a worker thread; callbacks (Then) run in Update().
        /// Missing or unreadable files fail the load instead of asserting.
        AsyncAsset<class Mesh>     LoadMeshAsync(PathView path);
        AsyncAsset<TextureAsset>   LoadTexture2DAsync(PathView path);

      private:
    };
}; // namespace Vortex
