/*
 * Created by v1tr10l7 on 29.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Asset/Asset.hpp>
#include <Vortex/Renderer/Export.hpp>

import Prism.Memory.Ref;
import Prism.Utility.Path;
import Vortex.RHI;

namespace Vortex
{
    /// An image file loaded into a sampled, mipmapped sRGB texture.
    class VT_RENDERER_API TextureAsset : public Asset
    {
      public:
        explicit TextureAsset(PathView filePath);
        virtual ~TextureAsset() = default;

        inline PathView            FilePath() const { return m_FilePath; }
        inline ::Ref<RHI::Texture> Texture() const { return m_Texture; }

      private:
        Path                m_FilePath;
        ::Ref<RHI::Texture> m_Texture;
    };
}; // namespace Vortex
