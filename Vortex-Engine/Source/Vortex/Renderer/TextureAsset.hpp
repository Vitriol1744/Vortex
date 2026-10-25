/*
 * Created by v1tr10l7 on 11.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Asset/Asset.hpp>
#include <Vortex/Renderer/RHI/Texture2D.hpp>

namespace Vortex
{
    class TextureAsset : public Asset
    {
      public:
        TextureAsset(PathView filePath);
        virtual ~TextureAsset() = default;

        inline PathView         FilePath() const { return m_FilePath; }
        inline ::Ref<Texture2D> Texture() const { return m_Texture; }

      private:
        Path             m_FilePath;
        ::Ref<Texture2D> m_Texture;
    };
}; // namespace Vortex
