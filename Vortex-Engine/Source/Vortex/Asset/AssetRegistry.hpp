/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/Memory/Ref.hpp>
#include <Vortex/Asset/Asset.hpp>

namespace Vortex
{
    class AssetRegistry
    {
      public:
        void                   Clear();

        inline constexpr usize Count() const { return m_Registry.Size(); }
        inline constexpr bool  Contains(const AssetHandle& handle) const
        {
            return m_Registry.Contains(handle);
        }

        const AssetMetadata& GetMetadata(const AssetHandle& handle) const;
        void                 RegisterAsset(const AssetHandle&   handle,
                                           const AssetMetadata& metadata);
        void                 Remove(const AssetHandle& handle);

        auto                 begin() { return m_Registry.begin(); }
        auto                 end() { return m_Registry.end(); }
        auto                 begin() const { return m_Registry.cbegin(); }
        auto                 end() const { return m_Registry.cend(); }

      private:
        UnorderedMap<AssetHandle, AssetMetadata> m_Registry;
    };
}; // namespace Vortex
