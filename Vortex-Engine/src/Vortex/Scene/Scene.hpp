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
#include "Vortex/Renderer/Camera.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

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
        Vec3   Translation = {0.0f, 0.0f, 0.0f};
        Vec3   Scale       = {1.0f, 1.0f, 1.0f};
        Quat   Rotation{};

        inline operator Mat4() const
        {
            return glm::translate(Mat4(1.0f), Translation)
                 * glm::mat4_cast(Rotation) * glm::scale(Mat4(1.0f), Scale);
        }
    };
    struct CameraComponent
    {
        class Camera Camera;
        bool         Primary          = true;
        bool         FixedAspectRatio = false;
    };
    struct SpriteComponent
    {
        Vec4 Color;
    };

    class VT_API Scene
    {
      public:
        explicit inline Scene(std::string_view name = "Unnamed")
            : m_Name(name)
        {
        }

        inline std::string_view GetName() const { return m_Name; }
        inline void            SetName(std::string_view name) { m_Name = name; }

        class Entity           AddEntity();
        void                   RemoveEntity(Entity entity);

        inline entt::registry& GetRegistry() { return m_Registry; }
        void                   DrawEntities();

        void                   Serialize(PathView path);
        void                   Deserialize(PathView path);

      private:
        std::string    m_Name = "Unnamed";
        entt::registry m_Registry;

        void DeserializeEntities(const nlohmann::ordered_json& entities);
        void DeserializeComponents(const nlohmann::ordered_json& components,
                                   Entity&                       entity);
    };
}; // namespace Vortex
