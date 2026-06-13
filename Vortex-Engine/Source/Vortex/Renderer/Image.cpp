/*
 * Created by v1tr10l7 on 15.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#define STBI_NO_SIMD 1
#define STB_IMAGE_IMPLEMENTATION
#include <Vortex/Renderer/Image.hpp>

namespace Vortex
{
    Image::Image(PathView path)
    {
        m_Pixels       = stbi_load(path.Raw(), (i32*)&m_Width, (i32*)&m_Height,
                                   (i32*)&m_Channels, STBI_rgb_alpha);

        m_BitsPerPixel = m_Channels * 8;
        m_Pitch        = m_Width * 4;

        VtCoreAssert(m_Pixels);
    }
    Image::~Image()
    {
        if (m_Pixels) stbi_image_free(m_Pixels);
        m_Pixels = nullptr;
    }
}; // namespace Vortex
