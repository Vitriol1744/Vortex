/*
 * Created by v1tr10l7 on 07.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Core.UUID;
import Prism.Memory.RefCounted;
import Prism.Utility.Path;
#include <Vortex/Asset/Export.hpp>

namespace Vortex
{
    using AssetHandle = UUID;

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Texture3D,
        Material,
        Mesh,
        Shader,
        AudioClip,
        Font,
        Script,
        Prefab,
        Animation,
        PhysicsMaterial,
        ParticleSystem,
        RenderPipeline,
        PostProcessingEffect
    };

    struct AssetMetadata
    {
        AssetHandle         Handle;
        AssetType           Type;
        Path                FilePath;
        Path                SourcePath;
        Vector<AssetHandle> Dependencies;
        bool                IsLoaded;
    };

    class VT_ASSET_API Asset : public RefCounted<Asset>
    {
      public:
        virtual ~Asset() {}

      private:
        AssetHandle m_Handle;
    };
}; // namespace Vortex
