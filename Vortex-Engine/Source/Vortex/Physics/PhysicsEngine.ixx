/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>

export module Vortex.Physics;

/**
 * Rigid-body physics on NVIDIA PhysX.
 *
 *   PhysicsEngine::Initialize();          // once, at startup
 *   PhysicsScene physics(scene);          // when a scene starts running
 *   physics.Simulate(deltaTime);          // every frame
 *
 * A PhysicsScene mirrors the actors of a Scene that have a RigidBodyComponent
 * and/or a collider (Box/SphereColliderComponent) when it is created. An
 * actor with only a collider is static. Simulate() steps PhysX at a fixed
 * rate, then writes the poses of dynamic bodies back to their transforms.
 *
 * Gameplay talks to bodies through their RigidBodyComponent (velocity, forces
 * and impulses are picked up on the next Simulate), so scripts never touch
 * PhysX. The interface stays free of glm and PhysX types: vectors are plain
 * float triples.
 */
export extern "C++"
{
    namespace Vortex
    {
        class Scene;

        struct PhysicsSettings
        {
            f32 Gravity[3]    = {0.0f, -9.81f, 0.0f};
            /// Seconds per simulation step.
            f32 FixedTimestep = 1.0f / 60.0f;
            /// Steps per Simulate() at most; a slower frame drops the rest
            /// rather than spiraling.
            u32 MaxSubsteps   = 8;
        };

        struct RaycastHit
        {
            /// entt id of the actor hit.
            u32 Entity      = ~0u;
            f32 Distance    = 0.0f;
            f32 Position[3] = {0.0f, 0.0f, 0.0f};
            f32 Normal[3]   = {0.0f, 0.0f, 0.0f};
        };

        namespace PhysicsEngine
        {
            bool             Initialize();
            void             Shutdown();
            bool             IsInitialized();

            /// Read by scenes as they are created.
            PhysicsSettings& Settings();
        } // namespace PhysicsEngine

        class PhysicsScene
        {
          public:
            explicit PhysicsScene(Scene& scene);
            ~PhysicsScene();

            PhysicsScene(const PhysicsScene&)            = delete;
            PhysicsScene& operator=(const PhysicsScene&) = delete;

            /// Advances by `deltaTime` seconds in fixed steps and syncs the
            /// transforms of dynamic bodies.
            void          Simulate(f32 deltaTime);

            /// The closest hit along the ray within `maxDistance`, skipping
            /// the actor `ignoredEntity` (e.g. the one casting it).
            bool          Raycast(const f32 origin[3], const f32 direction[3],
                                  f32 maxDistance, RaycastHit& outHit,
                                  u32 ignoredEntity = ~0u) const;

            /// Bodies created from the scene.
            usize         BodyCount() const;

          private:
            struct Impl;
            Impl* m_Impl = nullptr;
        };
    } // namespace Vortex
}
