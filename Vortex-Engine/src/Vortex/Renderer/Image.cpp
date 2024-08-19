/*
 * Created by v1tr10l7 on 15.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Vortex/Renderer/Image.hpp"

namespace Vortex
{
    Image::Image(PathView path)
    {
        int channels;
        m_Pixels = stbi_load(path.c_str(), (int*)&m_Width, (int*)&m_Height,
                             &channels, STBI_rgb_alpha);
        m_BitsPerPixel = channels * 8;

        VtCoreAssert(m_Pixels);
    }
}; // namespace Vortex
