/*
 * Created by v1tr10l7 on 24.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Math/Math.hpp>
#include <Vortex/Renderer/Export.hpp>

import Prism.Containers.Vector;
import Vortex.RHI;
import Vortex.Renderer.Material;

namespace Vortex
{
    /// Batched quads. Collected on the game thread, drawn on the render
    /// thread inside the scene's rendering:
    ///
    ///   Renderer2D::Begin();
    ///   Renderer2D::DrawQuad(transform, color); ...
    ///   auto batch = Renderer2D::End();          // a value; capture it
    ///   ... later, render thread:
    ///   Renderer2D::Draw(cmd, batch, camera);
    namespace Renderer2D
    {
        struct Vertex
        {
            Math::Vec3 Position;
            Math::Vec3 Color;
            Math::Vec2 TexCoords;
        };
        /// What End() hands over: the quads, and the sprite shader and its
        /// material sets as of then (so a reloaded shader applies from the
        /// next batch on).
        struct Batch
        {
            Vector<Vertex>             Vertices;
            ::Ref<RHI::Shader>         Shader;
            Vector<Material::BoundSet> Material;

            inline bool Empty() const { return Vertices.Empty(); }
        };

        /// Sets up sprites drawn into attachments of these formats.
        VT_RENDERER_API bool Initialize(RHI::Format colorFormat,
                                        RHI::Format depthFormat);
        VT_RENDERER_API void Shutdown();

        VT_RENDERER_API void Begin();
        VT_RENDERER_API void DrawQuad(const Mat4& transform, const Math::Vec4& color);
        inline void          DrawQuad(const Math::Vec2& pos, const Math::Vec2& scale,
                                      const Math::Vec4& color)
        {
            Mat4 transform = glm::translate(Mat4(1.0f), Math::Vec3(pos, 0.0f))
                           * glm::scale(Mat4(1.0f), Math::Vec3(scale, 1.0f));
            DrawQuad(transform, color);
        }
        /// The quads added since Begin.
        VT_RENDERER_API Batch End();
        /// Draws `batch` (render thread). `camera` is a uniform buffer whose
        /// first member is the view-projection matrix.
        VT_RENDERER_API void  Draw(RHI::CommandList& cmd, const Batch& batch,
                                   const ::Ref<RHI::Buffer>& camera);
    }; // namespace Renderer2D
}; // namespace Vortex
