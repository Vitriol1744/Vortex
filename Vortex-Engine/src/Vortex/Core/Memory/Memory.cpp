/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Memory/Memory.hpp"

namespace Vortex
{
    namespace Memory
    {
        static usize s_TotalAllocations   = 0;
        static usize s_TotalDeallocations = 0;

        void*        Allocate(usize bytes)
        {
            ++s_TotalAllocations;
            return malloc(bytes);
        }
        void* Reallocate(void* pointer, usize bytes)
        {
            return realloc(pointer, bytes);
        }
        void Free(void* pointer) noexcept
        {
            --s_TotalDeallocations;
            free(pointer);
        }

        usize GetAllocationCount() noexcept { return s_TotalAllocations; }
        usize GetDeallocationCount() noexcept { return s_TotalDeallocations; }
        usize GetUnfreedBlockCount() noexcept
        {
            return s_TotalAllocations - s_TotalDeallocations;
        }

    }; // namespace Memory
};     // namespace Vortex

using Vortex::usize;

void* operator new(usize size) { return Vortex::Memory::Allocate(size); }
void* operator new(usize size, std::align_val_t)
{
    return Vortex::Memory::Allocate(size);
}
void* operator new[](usize size) { return Vortex::Memory::Allocate(size); }
void* operator new[](usize size, std::align_val_t)
{
    return Vortex::Memory::Allocate(size);
}
void operator delete(void* ptr) noexcept { Vortex::Memory::Free(ptr); }
void operator delete(void* ptr, std::align_val_t) noexcept
{
    Vortex::Memory::Free(ptr);
}
void operator delete(void* ptr, usize) noexcept { Vortex::Memory::Free(ptr); }
void operator delete[](void* ptr) noexcept { Vortex::Memory::Free(ptr); }
void operator delete[](void* ptr, std::align_val_t) noexcept
{
    Vortex::Memory::Free(ptr);
}
void operator delete[](void* ptr, usize) noexcept { Vortex::Memory::Free(ptr); }
void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    Vortex::Memory::Free(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    Vortex::Memory::Free(ptr);
}
