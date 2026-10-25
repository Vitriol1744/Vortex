/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Span.hpp>
#include <Prism/Memory/Pointer.hpp>
#include <Vortex/Core/Export.hpp>

namespace Vortex
{
    enum class MemoryProtection
    {
        eNoAccess,
        eReadWrite,
    };

    namespace MemoryManager
    {
        PM_NODISCARD
        Span<u8> VT_CORE_API ReserveVirtualRange(usize size) noexcept;
        bool VT_CORE_API     CommitVirtualRange(
                u8* address, usize size, MemoryProtection protection) noexcept;

        void VT_CORE_API DecommitVirtualRange(u8* address, usize size) noexcept;
        void VT_CORE_API ReleaseVirtualRange(Span<u8> range) noexcept;
    }; // namespace MemoryManager
}; // namespace Vortex
