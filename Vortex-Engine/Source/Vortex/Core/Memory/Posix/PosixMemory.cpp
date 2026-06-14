/*
 * Created by v1tr10l7 on 14.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Memory/MemoryManager.hpp>

#include <sys/mman.h>
#include <unistd.h>

namespace Vortex
{
    namespace MemoryManager
    {
        static i32 MapProtection(MemoryProtection protection) noexcept
        {
            switch (protection)
            {
                case MemoryProtection::eNoAccess: return PROT_NONE;
                case MemoryProtection::eReadWrite:
                    return PROT_READ | PROT_WRITE;
            }
            return PROT_NONE;
        }

        Span<u8> ReserveVirtualRange(usize size) noexcept
        {
            void* ptr = ::mmap(nullptr, size, PROT_NONE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (ptr == MAP_FAILED) { return {}; }
            return {static_cast<u8*>(ptr), size};
        }

        bool CommitVirtualRange(u8* address, usize size,
                                MemoryProtection protection) noexcept
        {
            i32 result = ::mprotect(address, size, MapProtection(protection));
            return result == 0;
        }

        void DecommitVirtualRange(u8* address, usize size) noexcept
        {
            ::madvise(address, size, MADV_DONTNEED);
            ::mprotect(address, size, PROT_NONE);
        }

        void ReleaseVirtualRange(Span<u8> range) noexcept
        {
            if (range.Empty()) return;
            ::munmap(range.Raw(), range.Size());
        }

        Pointer Allocate(usize bytes)
        {
            auto pointer = mmap(nullptr, bytes, PROT_NONE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            return pointer == MAP_FAILED ? nullptr : pointer;
        }
        void Free(Pointer address, usize size) { munmap(address, size); }
    }; // namespace MemoryManager
}; // namespace Vortex
