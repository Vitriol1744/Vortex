/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorContext.hpp>
#include <Nexus/UI/Overlays/SettingsOverlay.hpp>
#include <Nexus/UI/Toolbar.hpp>

#include <Vortex/Core/Layer.hpp>

import Prism.Memory.Scope;

namespace Vortex::Editor
{
    /**
     * The editor as an application layer. It only wires the frame together;
     * the editor itself lives in:
     *
     *   EditorContext      shared state (scene, selection, camera, project)
     *   SceneController    scene files and play mode
     *   EditorCommands     every action, with its menu entry and shortcut
     *   UI/EditorWindows   every dockable window
     *   UI/MainMenuBar     menus, built from the commands and windows
     *   UI/Toolbar         play-mode buttons
     */
    class EditorLayer final : public Layer
    {
      public:
        EditorLayer()
            : Layer("Editor Layer")
        {
        }

        void OnAttach() override;
        void OnDetach() override;

        void OnUpdate(Timestep deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;

      private:
        Scope<EditorContext> m_Context;
        Scope<Toolbar>       m_Toolbar;
        SettingsOverlay      m_SettingsOverlay;

        /// Fullscreen host window of the dock space, with the menu bar and
        /// toolbar on top. Windows dock into it.
        void                 DrawDockspace();
    };
} // namespace Vortex::Editor
