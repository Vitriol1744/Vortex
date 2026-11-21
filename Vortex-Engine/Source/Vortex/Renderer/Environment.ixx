/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

export module Vortex.Renderer.Environment;

export import Vortex.RHI;

import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.Utility.PathView;

/**
 * An image-based lighting environment baked from an equirectangular HDR:
 *
 *  - Radiance:    the environment as a cube map with a full mip chain (the
 *                 skybox, and the source of the other two)
 *  - Irradiance:  diffuse lighting, a small cosine-convolved cube
 *  - Prefiltered: specular lighting, GGX-convolved per roughness, one
 *                 roughness per mip (split-sum approximation)
 *
 * plus the environment-independent BRDF lookup table that completes the split
 * sum. Baking runs on the GPU when the environment is loaded.
 */
export extern "C++"
{
    namespace Vortex
    {
        class VT_RENDERER_API Environment : public RefCounted<Environment>
        {
          public:
            /// Loads and bakes `hdrPath` (Radiance .hdr). Null on failure.
            static ::Ref<Environment> Load(PathView hdrPath);
            /// A constant-color environment (flat ambient light), for when
            /// no HDR is available.
            static ::Ref<Environment> Uniform(f32 r, f32 g, f32 b);

            /// The split-sum BRDF table (baked on first use).
            static ::Ref<RHI::Texture> BRDFLut();
            /// Frees the shared BRDF table; call before the renderer shuts
            /// down.
            static void                ReleaseShared();

            inline ::Ref<RHI::Texture> Radiance() const { return m_Radiance; }
            inline ::Ref<RHI::Texture> Irradiance() const { return m_Irradiance; }
            inline ::Ref<RHI::Texture> Prefiltered() const { return m_Prefiltered; }

            Environment() = default;

          private:
            ::Ref<RHI::Texture> m_Radiance;
            ::Ref<RHI::Texture> m_Irradiance;
            ::Ref<RHI::Texture> m_Prefiltered;
        };
    } // namespace Vortex
}
