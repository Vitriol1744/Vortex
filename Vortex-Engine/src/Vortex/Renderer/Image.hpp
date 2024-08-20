/*
 * Created by v1tr10l7 on 15.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

#include <stb_image.h>

namespace Vortex
{
    using Pixel = unsigned char;

    class VT_API Image
    {
      public:
        Image() = default;
        Image(PathView path);

        inline Pixel* GetPixels() const { return m_Pixels; }
        inline isize  GetSize() const
        {
            return m_Width * m_Height * m_BitsPerPixel;
        }

        inline isize GetWidth() const { return m_Width; }
        inline isize GetHeight() const { return m_Height; }
        inline isize GetBitsPerPixel() const { return m_BitsPerPixel; }

      private:
        Pixel* m_Pixels       = nullptr;

        isize  m_Width        = 0;
        isize  m_Height       = 0;
        isize  m_BitsPerPixel = 0;
    };
}; // namespace Vortex
