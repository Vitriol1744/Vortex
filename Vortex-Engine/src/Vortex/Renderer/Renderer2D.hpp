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
        void Initialize();
        void Shutdown();

        void BeginScene(Camera& camera);
        void EndScene();

        void DrawQuad(const Vec2& pos, const Vec2& scale, const Vec4& color);
    }; // namespace Renderer2D
};     // namespace Vortex
