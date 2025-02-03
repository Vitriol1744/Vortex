/*
 * Created by v1tr10l7 on 04.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    class VT_API VertexBuffer
    {
      public:
        virtual ~VertexBuffer() = default;

        virtual void  SetData(void* data, usize size, usize offset = 0) = 0;

        virtual usize GetSize() const                                   = 0;

        static Ref<VertexBuffer> Create(void* data, usize size);
    };
}; // namespace Vortex
