/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Asset/AssetRegistry.hpp>

namespace Vortex
{
    void AssetRegistry::Clear()
    {
        m_Registry.Clear();
    }

    const AssetMetadata& AssetRegistry::GetMetadata(const AssetHandle& handle) const
    {
        return m_Registry.At(handle);
    }

    void AssetRegistry::RegisterAsset(const AssetHandle&   handle,
                                     const AssetMetadata& metadata)
    {
        m_Registry[handle] = metadata;
    }

    void AssetRegistry::Remove(const AssetHandle& handle)
    {
        m_Registry.Erase(handle);
    }
}; // namespace Vortex
