/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Timestep.hpp>
#include <Vortex/Scene/Scene.hpp>

import Prism.Memory.Ref;
import Prism.Utility.Path;

namespace Vortex::Editor
{
    class EditorContext;

    enum class SceneState
    {
        /// Editing the authored scene.
        eEdit,
        /// A copy of it runs with scripts.
        ePlay,
        /// A copy of it runs physics and animations, without scripts.
        eSimulate,
    };

    /**
     * Owns the edited scene and its runtime copy: new/open/save, and the
     * play/simulate/pause/step/stop cycle. Play and Simulate never touch the
     * authored scene; Stop discards the copy.
     */
    class SceneController
    {
      public:
        explicit SceneController(EditorContext& context);

        // Edit-time scene, active outside Play/Simulate.
        inline ::Ref<Scene> EditorScene() const { return m_EditorScene; }
        inline PathView     ScenePath() const { return m_ScenePath; }
        inline SceneState   State() const { return m_State; }
        inline bool         IsRunning() const { return m_State != SceneState::eEdit; }
        inline bool         IsPaused() const { return m_Paused; }

        /// Adopts `scene` as the edited scene (stops a running one).
        void                SetEditorScene(::Ref<Scene> scene, PathView path = ""_pv);

        void                NewScene();
        /// Loaded on the next Update (not mid-frame).
        void                OpenScene(PathView path);
        void                OpenSceneDialog();
        /// Save to the current path, asking for one if there's none.
        bool                SaveScene();
        bool                SaveSceneAs();
        void                ReloadScene();

        void                Play();
        void                Simulate();
        void                Stop();
        void                SetPaused(bool paused);
        /// Advances a paused scene by `frames` fixed steps.
        void                Step(u32 frames = 1);

        /// Pending loads, then the active scene's update. Every frame.
        void                Update(Timestep deltaTime);

      private:
        EditorContext& m_Context;
        ::Ref<Scene>   m_EditorScene  = nullptr;
        ::Ref<Scene>   m_RuntimeScene = nullptr;
        Path           m_ScenePath    = "";
        Path           m_PendingPath  = "";
        SceneState     m_State        = SceneState::eEdit;
        bool           m_Paused       = false;
        u32            m_StepFrames   = 0;

        void           Start(SceneState state);
        void           Activate(::Ref<Scene> scene);
        bool           Load(PathView path);
    };
} // namespace Vortex::Editor
