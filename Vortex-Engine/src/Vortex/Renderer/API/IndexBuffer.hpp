/*
 * Created by v1tr10l7 on 05.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    class VT_API IndexBuffer
    {
      public:
        virtual ~IndexBuffer() = default;

        virtual void  SetData(u32* data, usize size, usize offset = 0) = 0;

        virtual usize GetSize() const                                  = 0;
        virtual u32   GetCount() const                                 = 0;

        static Ref<IndexBuffer> Create(u32* indices, usize size);
    };
}; // namespace Vortex
