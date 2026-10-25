/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Asset/AssetRegistry.hpp>

#include <mutex>

namespace Vortex
{
    void AssetRegistry::Clear()
    {
        std::unique_lock lock(m_Mutex);
        m_Registry.Clear();
    }

    const AssetMetadata& AssetRegistry::GetMetadata(const AssetHandle& handle) const
    {
        std::shared_lock lock(m_Mutex);
        return m_Registry.At(handle);
    }

    void AssetRegistry::RegisterAsset(const AssetHandle&   handle,
                                     const AssetMetadata& metadata)
    {
        std::unique_lock lock(m_Mutex);
        m_Registry[handle] = metadata;
    }

    void AssetRegistry::Remove(const AssetHandle& handle)
    {
        std::unique_lock lock(m_Mutex);
        m_Registry.Erase(handle);
    }
}; // namespace Vortex
