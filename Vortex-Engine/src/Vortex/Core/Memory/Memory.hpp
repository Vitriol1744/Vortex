/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Types.hpp"

namespace Vortex
{
    namespace Memory
    {
        VT_API void* Allocate(usize bytes);
        VT_API void* Reallocate(void* pointer, usize bytes);

        VT_API void  Free(void* pointer, usize bytes) noexcept;

        VT_API usize GetAllocationCount() noexcept;
        VT_API usize GetDeallocationCount() noexcept;
        VT_API usize GetUnfreedBlockCount() noexcept;

        template <typename T>
        T Allocate(usize bytes)
        {
            return reinterpret_cast<T>(Allocate(bytes));
        }
        template <typename T>
        T Reallocate(T pointer, usize bytes)
        {
            return reinterpret_cast<T>(Reallocate(pointer, bytes));
        }
        template <typename T>
        void Free(T pointer, usize bytes)
        {
            Free(reinterpret_cast<void*>(pointer), bytes);
        }
    } // namespace Memory
};    // namespace Vortex
