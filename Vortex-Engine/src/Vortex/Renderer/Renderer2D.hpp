/*
 * Created by v1tr10l7 on 24.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Renderer/Camera.hpp"

namespace Vortex
{
    namespace Renderer2D
    {
        void        Initialize();
        void        Shutdown();

        void        BeginScene(Camera& camera);
        void        EndScene();

        void        DrawQuad(const Mat4& transform, const Vec4& color);
        inline void DrawQuad(const Vec2& pos, const Vec2& scale,
                             const Vec4& color)
        {
            glm::mat4 transform
                = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(scale, 1.0f));

            DrawQuad(transform, color);
        }
    }; // namespace Renderer2D
};     // namespace Vortex
