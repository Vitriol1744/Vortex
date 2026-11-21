/*
 * Created by v1tr10l7 on 29.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/Image.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>

import Prism.String.Formatter;
import Vortex.Renderer;

namespace Vortex
{
    TextureAsset::TextureAsset(PathView filePath)
        : m_FilePath(filePath)
    {
        VtProfileFunction();
        Image image(filePath);
        if (!image.Pixels())
        {
            VtCoreError("TextureAsset: Failed to load '{}'", filePath);
            m_Texture = Renderer::WhiteTexture();
            return;
        }

        RHI::TextureDesc desc{};
        desc.Format    = RHI::Format::eRGBA8Srgb;
        desc.Extent    = {static_cast<u32>(image.Width()),
                          static_cast<u32>(image.Height()), 1};
        desc.MipLevels = RHI::FullMipChain(desc.Size2D());
        desc.DebugName = Prism::Format("{}", filePath);
        m_Texture = Renderer::Device().CreateTexture(desc, image.Pixels(),
                                                     image.Size());
    }
}; // namespace Vortex
