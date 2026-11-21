/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorCamera.hpp>
#include <Nexus/EditorPreferences.hpp>
#include <Nexus/EditorProject.hpp>
#include <Nexus/SceneController.hpp>
#include <Nexus/ScriptBuildService.hpp>

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Scene.hpp>

import Nexus.Commands;
import Nexus.EditorWindow;

namespace Vortex::Editor
{
    /**
     * Everything the editor's UI shares: the scene being edited and its
     * play state, the selection, the camera, preferences, commands and
     * windows. Windows and commands get it instead of reaching into the
     * EditorLayer or globals.
     */
    class EditorContext
    {
      public:
        explicit EditorContext(Application& app)
            : App(app)
            , Scenes(*this)
        {
        }
        EditorContext(const EditorContext&)            = delete;
        EditorContext& operator=(const EditorContext&) = delete;

        Application&       App;
        EditorPreferences  Preferences;
        EditorCamera       Camera;
        /// Lives in the preferences, so it's saved with them.
        ViewportSettings&  Viewport = Preferences.Viewport;
        SceneController    Scenes;
        ScriptBuildService Scripts;
        CommandRegistry    Commands;
        WindowManager      Windows;
        bool               ShowSettings = false;

        /// The scene shown and edited right now (the runtime copy while
        /// playing).
        inline Scene&      ActiveScene() const { return *Scene::Active(); }

        // Selection --------------------------------------------------------
        inline Actor       Selection() const { return m_Selection; }
        inline void        Select(entt::entity entity)
        {
            m_Selection = entity == entt::null ? Actor{}
                                               : Actor(entity, ActiveScene());
        }
        inline void        ClearSelection() { m_Selection = Actor{}; }
        /// Re-binds the selection to the active scene (same entity id), or
        /// drops it if the entity doesn't exist there.
        inline void        RetargetSelection()
        {
            entt::entity entity = m_Selection;
            if (entity != entt::null && ActiveScene().Registry().valid(entity))
                m_Selection = Actor(entity, ActiveScene());
            else m_Selection = Actor{};
        }

        // Projects ---------------------------------------------------------
        /// Makes `project` the active one at the start of the next update
        /// (never mid-frame).
        void               OpenProject(::Ref<EditorProject> project);
        void               OpenProjectDialog();
        /// The preferences' startup project, if there is one.
        bool               OpenStartupProject();

        // Preferences ------------------------------------------------------
        /// Reads/writes editorPreferences.json.
        bool               LoadPreferences();
        void               SavePreferences() const;
        /// Applies the preferences to the editor (theme, UI scale, VSync,
        /// shortcuts). Deferred to the next Update, outside the ImGui frame.
        void               ApplyPreferences() { m_ApplyPending = true; }
        /// Copies the rebound shortcuts into the preferences.
        void               StoreShortcuts();

        /// Pending project, script builds, the scene, then every window.
        void               Update(f32 deltaTime);

        /// The ProjectChangedEvent handler: remembers the project, loads its
        /// start scene and tells the windows.
        void               OnProjectChanged(const ProjectChangedEvent& event);

      private:
        Actor                m_Selection{};
        ::Ref<EditorProject> m_PendingProject = nullptr;
        bool                 m_ApplyPending   = false;
        f32                  m_AutoSaveTimer  = 0.0f;

        void                 ApplyPendingPreferences();
        void                 UpdateAutoSave(f32 deltaTime);
    };
} // namespace Vortex::Editor
