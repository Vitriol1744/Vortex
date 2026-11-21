/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorCommands.hpp>
#include <Nexus/EditorContext.hpp>
#include <Nexus/UI/Modals/NewProjectModal.hpp>

#include <Vortex/Renderer/ImGui/ModalManager.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

#include <imgui.h>

import Nexus.TestScenes;
import Vortex.Renderer;

namespace Vortex::Editor
{
    namespace
    {
        constexpr int Ctrl  = ImGuiMod_Ctrl;
        constexpr int Shift = ImGuiMod_Shift;
        constexpr int Alt   = ImGuiMod_Alt;

        void RegisterFileCommands(EditorContext& context)
        {
            auto& commands = context.Commands;
            auto& scenes   = context.Scenes;

            commands.Register({.Id       = "Scene.New",
                               .Label    = "New Scene",
                               .Menu     = "File",
                               .Shortcut = Ctrl | ImGuiKey_N,
                               .Execute  = [&scenes] { scenes.NewScene(); }});
            commands.Register({.Id      = "Scene.NewPhysicsTest",
                               .Label   = "New Physics Test Scene",
                               .Menu    = "File",
                               .Execute = [&context]
                               {
                                   context.Scenes.SetEditorScene(CreatePhysicsTestScene());
                                   // Behind the player, looking at the pyramid.
                                   context.Camera.SetPosition({0.0f, 4.0f, 14.0f});
                               }});
            commands.Register({.Id      = "Scene.NewFpsTest",
                               .Label   = "New FPS Test Scene",
                               .Menu    = "File",
                               .Execute = [&context]
                               {
                                   context.Scenes.SetEditorScene(CreateFpsTestScene());
                                   // Overlooking the courtyard from its east end.
                                   context.Camera.SetPosition({11.0f, 3.0f, 0.0f});
                               }});
            commands.Register({.Id       = "Scene.Open",
                               .Label    = "Open Scene...",
                               .Menu     = "File",
                               .Shortcut = Ctrl | ImGuiKey_O,
                               .Execute  = [&scenes] { scenes.OpenSceneDialog(); }});
            commands.Register({.Id       = "Scene.Save",
                               .Label    = "Save Scene",
                               .Menu     = "File",
                               .Shortcut = Ctrl | ImGuiKey_S,
                               .Execute  = [&scenes] { scenes.SaveScene(); }});
            commands.Register({.Id       = "Scene.SaveAs",
                               .Label    = "Save Scene As...",
                               .Menu     = "File",
                               .Shortcut = Ctrl | Shift | ImGuiKey_S,
                               .Execute  = [&scenes] { scenes.SaveSceneAs(); }});
            commands.Register({.Id       = "Scene.Reload",
                               .Label    = "Reload Scene",
                               .Menu     = "File",
                               .Shortcut = Ctrl | ImGuiKey_R,
                               .Execute  = [&scenes] { scenes.ReloadScene(); }});

            commands.Register(
                {.Id       = "Project.New",
                 .Label    = "New Project...",
                 .Menu     = "File",
                 .Group    = 1,
                 .Shortcut = Ctrl | Shift | ImGuiKey_N,
                 .Execute  = [&context]
                 {
                     IgnoreUnused(UI::ModalManager::Open<NewProjectModal>(
                         [&context](const ProjectConfig& config, PathView path,
                                    StringView templateId, bool gitIgnore)
                         {
                             auto project = EditorProject::NewFromTemplate(
                                 config, path, templateId, gitIgnore);
                             if (!project)
                                 VtCoreErrorTag("Editor", "Can't create the project");
                             context.OpenProject(project);
                         }));
                 }});
            commands.Register({.Id       = "Project.Open",
                               .Label    = "Open Project...",
                               .Menu     = "File",
                               .Group    = 1,
                               .Shortcut = Ctrl | Alt | ImGuiKey_O,
                               .Execute = [&context] { context.OpenProjectDialog(); }});

            auto& app = context.App;
            commands.Register({.Id       = "Editor.Restart",
                               .Label    = "Restart",
                               .Menu     = "File",
                               .Group    = 2,
                               .Shortcut = Alt | ImGuiKey_F5,
                               .Execute  = [&app] { app.Restart(); }});
            commands.Register({.Id       = "Editor.Quit",
                               .Label    = "Quit",
                               .Menu     = "File",
                               .Group    = 2,
                               .Shortcut = Ctrl | ImGuiKey_Q,
                               .Execute  = [&app] { app.Close(); }});
        }

        void RegisterEditCommands(EditorContext& context)
        {
            context.Commands.Register(
                {.Id        = "Editor.Settings",
                 .Label     = "Settings",
                 .Menu      = "Edit",
                 .Shortcut  = Ctrl | ImGuiKey_Comma,
                 .Execute   = [&context] { context.ShowSettings = true; },
                 .IsChecked = [&context] { return context.ShowSettings; }});
            context.Commands.Register(
                {.Id        = "Viewport.ToggleGrid",
                 .Label     = "Show Grid",
                 .Menu      = "View",
                 .Shortcut  = Ctrl | ImGuiKey_G,
                 .Execute   = [&context]
                 { context.Viewport.ShowGrid = !context.Viewport.ShowGrid; },
                 .IsChecked = [&context] { return context.Viewport.ShowGrid; }});
        }

        void RegisterSceneCommands(EditorContext& context)
        {
            auto& commands  = context.Commands;
            auto& scenes    = context.Scenes;
            auto  isRunning = [&scenes] { return scenes.IsRunning(); };

            commands.Register({.Id         = "Scene.Play",
                               .Label      = "Play",
                               .Menu       = "Scene",
                               .Shortcut   = ImGuiKey_F5,
                               .Execute    = [&scenes] { scenes.Play(); },
                               .CanExecute = [&scenes] { return !scenes.IsRunning(); }});
            commands.Register({.Id         = "Scene.Simulate",
                               .Label      = "Simulate",
                               .Menu       = "Scene",
                               .Shortcut   = Ctrl | ImGuiKey_F5,
                               .Execute    = [&scenes] { scenes.Simulate(); },
                               .CanExecute = [&scenes] { return !scenes.IsRunning(); }});
            commands.Register({.Id         = "Scene.Stop",
                               .Label      = "Stop",
                               .Menu       = "Scene",
                               .Shortcut   = Shift | ImGuiKey_F5,
                               .Execute    = [&scenes] { scenes.Stop(); },
                               .CanExecute = isRunning});
            commands.Register(
                {.Id         = "Scene.Pause",
                 .Label      = "Pause",
                 .Menu       = "Scene",
                 .Group      = 1,
                 .Shortcut   = ImGuiKey_F6,
                 .Execute    = [&scenes] { scenes.SetPaused(!scenes.IsPaused()); },
                 .CanExecute = isRunning,
                 .IsChecked  = [&scenes] { return scenes.IsPaused(); }});
            commands.Register({.Id         = "Scene.Step",
                               .Label      = "Step Frame",
                               .Menu       = "Scene",
                               .Group      = 1,
                               .Shortcut   = ImGuiKey_F10,
                               .Execute    = [&scenes] { scenes.Step(); },
                               .CanExecute = [&scenes]
                               { return scenes.IsRunning() && scenes.IsPaused(); }});
        }

        void RegisterToolCommands(EditorContext& context)
        {
            auto& commands = context.Commands;
            auto& scripts  = context.Scripts;

            commands.Register({.Id         = "Scripts.Build",
                               .Label      = "Build C# Scripts",
                               .Menu       = "Tools",
                               .Shortcut   = Ctrl | ImGuiKey_B,
                               .Execute    = [&scripts] { scripts.Build(); },
                               .CanExecute = [&scripts] { return !scripts.IsBuilding(); }});
            // Pipelines are made on demand from the shaders, so a reload is
            // all it takes for the next frame to use the new code.
            commands.Register({.Id       = "Renderer.ReloadShaders",
                               .Label    = "Reload Shaders",
                               .Menu     = "Tools",
                               .Shortcut = Ctrl | Shift | ImGuiKey_R,
                               .Execute  = [] { Renderer::ShaderLibrary().ReloadAll(); }});
        }
    } // namespace

    void RegisterEditorCommands(EditorContext& context)
    {
        RegisterFileCommands(context);
        RegisterEditCommands(context);
        RegisterSceneCommands(context);
        RegisterToolCommands(context);
    }
} // namespace Vortex::Editor
