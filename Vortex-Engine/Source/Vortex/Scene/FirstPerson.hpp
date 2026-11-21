/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Types.hpp>
#include <Vortex/Scene/Export.hpp>

namespace Vortex
{
    class Scene;
    class Camera;

    /**
     * The built-in first-person character (FirstPersonControllerComponent +
     * WeaponComponent). The scene runs it in Play mode:
     *
     *   Update      input, mouse look, movement, jumping and shooting
     *   (physics)
     *   LateUpdate  camera, weapon model, muzzle flash and bullet holes follow
     *               the bodies that just moved
     */
    namespace FirstPerson
    {
        void VT_SCENE_API    Update(Scene& scene, f32 deltaTime);
        void VT_SCENE_API    LateUpdate(Scene& scene, f32 deltaTime);
        /// Play stopped: gives the mouse back.
        void VT_SCENE_API    Stop(Scene& scene);

        /// The camera of the scene's first-person character, if it has one;
        /// what Play mode should render with.
        Camera* VT_SCENE_API ActiveCamera(Scene& scene, f32* outFieldOfView = nullptr);

        /// Whether a click may capture the mouse. The editor allows it only
        /// over its game viewport (default: anywhere in the window).
        void VT_SCENE_API    SetCaptureAllowed(bool allowed);
        bool VT_SCENE_API    IsMouseCaptured();
    } // namespace FirstPerson
} // namespace Vortex
