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
    struct TagComponent
    {
        std::string Name = "";

        TagComponent()   = default;
        TagComponent(std::string name)
            : Name(name)
        {
        }
        TagComponent(const char* name)
            : Name(name)
        {
        }

        inline operator const std::string&() const { return Name; }
        inline operator const char*() const { return Name.data(); }
    };
    struct TransformComponent
    {
        Vec3   Translation;
        Vec3   Scale;
        Quat   Rotation;

        inline operator Mat4() const
        {
            return glm::translate(Mat4(1.0f), Translation)
                 * glm::mat4_cast(Rotation) * glm::scale(Mat4(1.0f), Scale);
        }
    };
    struct SpriteComponent
    {
        Vec4 Color;
    };

    class VT_API Scene
    {
      public:
        Scene() = default;

        class Entity           AddEntity();

        inline entt::registry& GetRegistry() { return m_Registry; }
        void                   DrawEntities();

        void                   Serialize();
        void                   Deserialize();

      private:
        entt::registry m_Registry;
    };
}; // namespace Vortex
