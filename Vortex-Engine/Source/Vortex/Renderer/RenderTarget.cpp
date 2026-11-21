/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Types.hpp>

import Prism.String.Formatter;
import Vortex.Renderer;
import Vortex.Renderer.RenderTarget;

namespace Vortex
{
    //------------------------------------------------------------------------
    // SceneRenderTarget -----------------------------------------------------
    //------------------------------------------------------------------------

    SceneRenderTarget::SceneRenderTarget(RHI::Extent2D size, StringView name)
        : m_Name(name)
    {
        Resize(size);
    }

    void SceneRenderTarget::Resize(RHI::Extent2D size)
    {
        if (size.Width == 0) size.Width = 1;
        if (size.Height == 0) size.Height = 1;
        if (m_Color && size == m_Size) return;
        m_Size = size;

        auto&            device = Renderer::Device();
        RHI::TextureDesc color{};
        color.Format = Renderer::kSceneColorFormat;
        color.Extent = {size.Width, size.Height, 1};
        color.Usage
            = RHI::TextureUsage::eColorAttachment | RHI::TextureUsage::eSampled;
        color.DebugName = Prism::Format("{} color", m_Name);
        m_Color         = device.CreateTexture(color);

        RHI::TextureDesc depth{};
        depth.Format    = Renderer::kSceneDepthFormat;
        depth.Extent    = {size.Width, size.Height, 1};
        depth.Usage     = RHI::TextureUsage::eDepthStencilAttachment;
        depth.DebugName = Prism::Format("{} depth", m_Name);
        m_Depth         = device.CreateTexture(depth);
    }
} // namespace Vortex
