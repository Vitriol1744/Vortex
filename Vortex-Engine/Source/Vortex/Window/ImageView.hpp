/*
 * Created by v1tr10l7 on 05.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Vortex/Core/Export.hpp>

namespace Vortex
{
    using Pixel = unsigned char;

    class VT_API ImageView
    {
      public:
        inline constexpr ImageView(Pixel* pixels, isize width, isize height,
                                   isize pitch, isize channels,
                                   isize bitsPerPixel)
            : m_Pixels(pixels)
            , m_Width(width)
            , m_Height(height)
            , m_Pitch(pitch)
            , m_Channels(channels)
            , m_BitsPerPixel(bitsPerPixel)
        {
        }

        inline Pixel* Pixels() const { return m_Pixels; }
        inline isize  Size() const { return m_Width * m_Height * 4; }

        inline isize  Width() const { return m_Width; }
        inline isize  Height() const { return m_Height; }
        inline isize  Pitch() const { return m_Pitch; }
        inline isize  ChannelCount() const { return m_Channels; }
        inline isize  BitsPerPixel() const { return m_BitsPerPixel; }

      private:
        Pixel* m_Pixels       = nullptr;

        isize  m_Width        = 0;
        isize  m_Height       = 0;
        isize  m_Pitch        = 0;
        isize  m_Channels     = 0;
        isize  m_BitsPerPixel = 0;
    };
} // namespace Vortex
