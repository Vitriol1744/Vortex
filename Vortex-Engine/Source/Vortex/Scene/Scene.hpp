/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

import Prism.Utility.Path;
#include <Vortex/Core/Export.hpp>
#include <Vortex/Core/Math/Matrix.hpp>
#include <Vortex/Core/Math/Quaternion.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Core/Time.hpp>

#include <Vortex/Scene/Camera.hpp>
#include <Vortex/Scene/Export.hpp>
#include <Vortex/Serialization/Archive.hpp>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace Vortex
{
    class PhysicsScene;

    class VT_SCENE_API Scene : public RefCounted<Scene>
    {
      public:
        explicit inline Scene(StringView name = "Unnamed"_sv)
            : m_Name(name)
        {
        }
        ~Scene();

        Scene(const Scene&)            = delete;
        Scene& operator=(const Scene&) = delete;

        inline StringView Name() const { return m_Name; }
        inline void       SetName(StringView name) { m_Name = name; }

        class Actor       AddActor();
        void              RemoveActor(Actor actor);

        /// Play: creates the physics world and the script instances.
        void              OnRuntimeStart();
        /// Simulate: physics only, no scripts.
        void              OnSimulationStart();
        /// Ends Play or Simulate: destroys script instances and physics.
        void              OnRuntimeStop();

        /// Edit time: animations only.
        void              Update(Timestep ts);
        /// Play: scripts, then physics, then animations.
        void              UpdateRuntime(Timestep ts);
        /// Simulate: physics, then animations.
        void              UpdateSimulation(Timestep ts);
        /// Advances every AnimationComponent and computes its pose.
        void              UpdateAnimations(Timestep ts);

        /// The physics world while the scene runs, null otherwise.
        inline PhysicsScene* Physics() const { return m_Physics; }

        inline Camera&    MainCamera() { return *m_MainCamera; }
        inline void SetMainCamera(Camera& camera) { m_MainCamera = &camera; }

        inline entt::registry&       Registry() { return m_Registry; }
        inline const entt::registry& Registry() const { return m_Registry; }

        void                         Serialize(ArchiveWriter& writer) const;
        bool                         Deserialize(ArchiveReader& reader);

        inline static ::Ref<Scene>   Active() { return s_Active; }
        inline static void SwitchTo(::Ref<Scene> scene) { s_Active = scene; }

      private:
        String                     m_Name       = "Unnamed";
        Camera*                    m_MainCamera = nullptr;
        entt::registry             m_Registry;
        PhysicsScene*              m_Physics    = nullptr;
        bool                       m_ScriptsRunning = false;

        void                       StartPhysics();
        void                       StopPhysics();

        inline static ::Ref<Scene> s_Active = nullptr;
    };
}; // namespace Vortex
