/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Asset/AssetManager.hpp>

#include <mutex>

namespace Vortex
{
    AssetManager::~AssetManager()
    {
        m_Registry->Clear();
    }

    void AssetManager::Update() { m_Loader.Update(); }

    void AssetManager::UnloadAsset(const AssetHandle& handle)
    {
        std::unique_lock lock(m_Mutex);
        m_AssetCache.Erase(handle);
    }
}; // namespace Vortex
