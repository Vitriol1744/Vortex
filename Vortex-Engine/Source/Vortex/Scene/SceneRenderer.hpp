/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// A header rather than a module: its interface is glm and scene types, and
// clang 23 crashes (ASTReader::UpdateDecl) importing a module that carries
// them in its global module fragment.

#include <Vortex/Core/Math/Matrix.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Scene/Export.hpp>

import Prism.Memory.Ref;
import Prism.Utility.PathView;
import Vortex.RHI;
import Vortex.Renderer.Environment;
import Vortex.Renderer.RenderGraph;
import Vortex.Renderer.RenderTarget;

namespace Vortex
{
    class Camera;
    class Scene;

    /**
     * Renders a Scene as seen by a camera into a SceneRenderTarget, as render
     * graph passes:
     * PBR meshes (static and skinned) lit by a directional light and the
     * environment (image-based lighting), the environment as a skybox, an
     * optional ground grid, then sprites. Any number of views per frame is fine (editor viewport, game
     * view, thumbnails): each gets its own camera data.
     */
    namespace SceneRenderer
    {
        struct Settings
        {
            /// Direction the sunlight travels.
            Math::Vec3 LightDirection       = {-0.4f, -1.0f, -0.3f};
            Math::Vec3 LightColor           = {1.0f, 0.96f, 0.9f};
            f32        LightIntensity       = 3.0f;
            f32        Exposure             = 1.0f;
            f32        EnvironmentIntensity = 1.0f;
            /// Mip of the radiance map the skybox shows (0 = sharp).
            f32        SkyboxBlur           = 0.0f;
        };

        /// Per-view extras (editor overlays).
        struct ViewOptions
        {
            /// An infinite grid on the XZ plane, under the scene's geometry.
            bool DrawGrid = false;
        };

        VT_SCENE_API bool Initialize();
        VT_SCENE_API void Shutdown();

        VT_SCENE_API Settings&  GetSettings();

        /// The environment lighting every view (and drawn as the skybox).
        VT_SCENE_API void       SetEnvironment(::Ref<Environment> environment);
        VT_SCENE_API ::Ref<Environment> GetEnvironment();
        /// Loads and bakes an HDR; keeps the current environment on failure.
        VT_SCENE_API bool       LoadEnvironment(PathView hdrPath);

        /// The surface shader for static meshes ("PBR"), for materials.
        VT_SCENE_API ::Ref<RHI::Shader> Shader();

        /// Adds the passes rendering `scene` through `camera` into `target`.
        /// Snapshots the scene now (game thread); the passes record from the
        /// snapshot on the render thread. Leaves the color in eShaderRead.
        VT_SCENE_API void AddToGraph(RenderGraph& graph, Scene& scene,
                                     Camera& camera, SceneRenderTarget& target,
                                     const ViewOptions& options = {});
    } // namespace SceneRenderer
} // namespace Vortex
