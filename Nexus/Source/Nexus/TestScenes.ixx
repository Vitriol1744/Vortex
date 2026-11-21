/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

export module Nexus.TestScenes;

import Prism.Memory.Ref;

/// Scenes built in code to exercise engine features.
export extern "C++"
{
    namespace Vortex
    {
        class Scene;
    } // namespace Vortex

    namespace Vortex::Editor
    {
        /**
         * Physics playground: a static ground and ramp, a pyramid of dynamic
         * crates, bouncing balls and a player (Gameplay.PlayerController).
         * Simulate drops everything; Play also runs the player script
         * (WASD/arrows move, Shift sprints, Space jumps).
         *
         * Its cube and sphere meshes load asynchronously and appear once
         * ready; the bodies don't depend on them.
         */
        ///
        /// `preloadMeshes` loads the cube and sphere before returning, so a
        /// scene that's saved straight away (a project template) records them.
        ::Ref<Scene> CreatePhysicsTestScene(bool preloadMeshes = false);

        /**
         * First-person shooter playground in Sponza: Play, click the viewport
         * and walk (WASD, Space jumps, Shift sprints), look with the mouse and
         * shoot with the left button. Shots leave bullet holes and push
         * crates, balls and the box ragdolls around. Esc frees the mouse.
         *
         * Sponza loads synchronously (its collision is cooked from it); its
         * textures stream in.
         */
        ::Ref<Scene> CreateFpsTestScene();
    } // namespace Vortex::Editor
}
