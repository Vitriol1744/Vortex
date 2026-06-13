/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/Path.hpp>
#include <Vortex/Core/Export.hpp>
#include <Vortex/Core/Math/Matrix.hpp>
#include <Vortex/Core/Math/Quaternion.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Core/Time.hpp>
#include <Vortex/Renderer/API/GraphicsPipeline.hpp>
#include <Vortex/Renderer/Camera.hpp>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace Vortex
{
    class VT_API Scene
    {
      public:
        explicit inline Scene(StringView name = "Unnamed"_sv)
            : m_Name(name)
        {
        }

        inline StringView Name() const { return m_Name; }
        inline void       SetName(StringView name) { m_Name = name; }

        class Entity      AddEntity();
        void              RemoveEntity(Entity entity);

        void              Update(Timestep ts);
        void              Render();

        inline Camera&    MainCamera() { return *m_MainCamera; }
        inline void SetMainCamera(Camera& camera) { m_MainCamera = &camera; }

        inline entt::registry& Registry() { return m_Registry; }

        void                   Serialize(PathView path);
        void                   Deserialize(PathView path);

      private:
        String         m_Name       = "Unnamed";
        Camera*        m_MainCamera = nullptr;
        entt::registry m_Registry;

        void DeserializeEntities(const nlohmann::ordered_json& entities);
        void DeserializeComponents(const nlohmann::ordered_json& components,
                                   Entity&                       entity);
    };
}; // namespace Vortex
