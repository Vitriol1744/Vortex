/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Renderer/Image.hpp"

namespace Vortex
{
    class VT_API Texture2D
    {
      public:
        Texture2D(PathView path) { m_Image = Image(path); }
        virtual ~Texture2D()                     = default;

        virtual u32           GetMipLevelCount() = 0;

        static Ref<Texture2D> Create(PathView path);

      protected:
        Image m_Image;
    };
}; // namespace Vortex
