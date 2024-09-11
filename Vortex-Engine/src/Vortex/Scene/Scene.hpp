/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Export.hpp"
#include "Vortex/Core/Math/Matrix.hpp"
#include "Vortex/Core/Math/Quaternion.hpp"
#include "Vortex/Core/Math/Vector.hpp"

#include <entt/entt.hpp>

namespace Vortex
{
    class Entity;
    struct TransformComponent
    {
        Mat4   Transform;

        inline operator const Mat4&() const { return Transform; }
    };
    struct SpriteComponent
    {
        Vec4 Color;
    };

    class VT_API Scene
    {
      public:
        Scene() = default;

        Entity                 AddEntity();

        inline entt::registry& GetRegistry() { return m_Registry; }
        void                   DrawEntities();

      private:
        entt::registry m_Registry;
    };
}; // namespace Vortex
