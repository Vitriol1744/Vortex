/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

export module Vortex.Renderer.RenderTarget;

export import Vortex.RHI;

import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;
import Prism.String.StringView;

export extern "C++"
{
    namespace Vortex
    {
        /// Color and depth textures a view renders into. The color texture
        /// rests in eShaderRead, ready to be shown (e.g. by ImGui).
        class VT_RENDERER_API SceneRenderTarget
            : public RefCounted<SceneRenderTarget>
        {
          public:
            SceneRenderTarget(RHI::Extent2D size, StringView name);

            /// Recreates the textures if the size changed. The old ones stay
            /// alive until the GPU no longer uses them.
            void Resize(RHI::Extent2D size);

            inline RHI::Extent2D       Size() const { return m_Size; }
            inline ::Ref<RHI::Texture> Color() const { return m_Color; }
            inline ::Ref<RHI::Texture> Depth() const { return m_Depth; }

          private:
            String              m_Name{};
            RHI::Extent2D       m_Size{};
            ::Ref<RHI::Texture> m_Color;
            ::Ref<RHI::Texture> m_Depth;
        };
    } // namespace Vortex
}
