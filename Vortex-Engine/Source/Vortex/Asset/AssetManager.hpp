/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Types.hpp>

#include <Vortex/Asset/AssetRegistry.hpp>

import Vortex.Asset.AsyncLoader;

namespace Vortex
{
    class VT_ASSET_API AssetManager : public RefCounted<AssetManager>
    {
      public:
        AssetManager() { m_Registry = CreateRef<AssetRegistry>(); }
        virtual ~AssetManager();

        virtual ::Ref<class Asset> Asset(const AssetHandle& handle) = 0;
        virtual void               UnloadAsset(const AssetHandle& handle);

        PM_NODISCARD
        inline class AssetRegistry& Registry() { return *m_Registry; }

        /// Background loads (see Vortex.Asset.AsyncLoader).
        inline AsyncAssetLoader&    Loader() { return m_Loader; }

        /// Main thread, once a frame: completion callbacks of async loads.
        void                        Update();

      protected:
        AsyncAssetLoader                              m_Loader;
        ::Ref<AssetRegistry>                          m_Registry;
        UnorderedMap<AssetHandle, ::Ref<class Asset>> m_AssetCache;
        mutable std::shared_mutex                     m_Mutex;
    }; // namespace AssetManager
}; // namespace Vortex
