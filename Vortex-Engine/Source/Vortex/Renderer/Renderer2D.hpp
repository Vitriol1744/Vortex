/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Math/Math.hpp>

namespace Vortex
{
    namespace Renderer2D
    {
        void        Initialize(::Ref<class Framebuffer> framebuffer);
        void        Shutdown();

        void        BeginScene(const Mat4& viewProjection);
        void        EndScene();

        void        DrawQuad(const Mat4& transform, const Math::Vec4& color);
        inline void DrawQuad(const Math::Vec2& pos, const Math::Vec2& scale,
                             const Math::Vec4& color)
        {
            Mat4 transform = glm::translate(Mat4(1.0f), Math::Vec3(pos, 0.0f))
                           * glm::scale(Mat4(1.0f), Math::Vec3(scale, 1.0f));

            DrawQuad(transform, color);
        }
    }; // namespace Renderer2D
}; // namespace Vortex
