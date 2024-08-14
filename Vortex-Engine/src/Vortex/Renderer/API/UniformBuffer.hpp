/*
 * Created by v1tr10l7 on 07.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    class VT_API UniformBuffer
    {
      public:
        virtual ~UniformBuffer() = default;

        virtual void SetData(const void* data, usize size, usize offset = 0)
            = 0;

        static Ref<UniformBuffer> Create(usize size);
    };
}; // namespace Vortex
