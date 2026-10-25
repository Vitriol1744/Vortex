/*
 * Created by v1tr10l7 on 11.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/Image.hpp>
#include <Vortex/Renderer/RHI/RHI.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>

namespace Vortex
{
    TextureAsset::TextureAsset(PathView filePath)
        : m_FilePath(filePath)
    {
        Image image(filePath);
        m_Texture = RHI::CreateTexture2D(image);
    }
}; // namespace Vortex
