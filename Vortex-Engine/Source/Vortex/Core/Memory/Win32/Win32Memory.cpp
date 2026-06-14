/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Memory/MemoryManager.hpp>
#include <Vortex/Core/Win32/Windows.hpp>

namespace Vortex
{
    namespace MemoryManager
    {
        static DWORD MapProtection(MemoryProtection protection) noexcept
        {
            switch (protection)
            {
                case MemoryProtection::eNoAccess: return PAGE_NOACCESS;
                case MemoryProtection::eReadWrite: return PAGE_READWRITE;
            }
            return PAGE_NOACCESS;
        }

        Span<u8> ReserveVirtualRange(usize size) noexcept
        {
            void* ptr
                = ::VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
            if (!ptr) { return {}; }
            return {static_cast<u8*>(ptr), size};
        }

        bool CommitVirtualRange(u8* address, usize size,
                                MemoryProtection protection) noexcept
        {
            void* ptr = ::VirtualAlloc(address, size, MEM_COMMIT,
                                       MapProtection(protection));
            return ptr != nullptr;
        }

        void DecommitVirtualRange(u8* address, usize size) noexcept
        {
            ::VirtualFree(address, size, MEM_DECOMMIT);
        }
        void ReleaseVirtualRange(Span<u8> range) noexcept
        {
            if (range.Empty()) return;
            ::VirtualFree(range.Raw(), 0, MEM_RELEASE);
        }
    }; // namespace MemoryManager
}; // namespace Vortex
