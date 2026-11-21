/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorContext.hpp>

#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>
#include <Vortex/Window/Window.hpp>

#include <imgui.h>

import Prism.Containers.Array;
import Vortex.Renderer;

namespace Vortex::Editor
{
    namespace
    {
        constexpr StringView kPreferencesPath = EditorPreferences::FileName;

        auto s_ProjectFilter
            = ToArray<Window::FileFilter>({{"Vortex Project file", "vproj"}});

        void ApplyAppearance(EditorTheme theme, f32 scale)
        {
            // The style as the ImGui layer set it up (Vortex Dark, unscaled):
            // every theme and scale starts from it, so nothing compounds.
            static const ImGuiStyle s_BaseStyle = ImGui::GetStyle();

            ImGuiStyle style = s_BaseStyle;
            switch (theme)
            {
                case EditorTheme::eImGuiDark: ImGui::StyleColorsDark(&style); break;
                case EditorTheme::eLight: ImGui::StyleColorsLight(&style); break;
                case EditorTheme::eClassic: ImGui::StyleColorsClassic(&style); break;
                default: break;
            }
            // Platform windows must look like regular ones.
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding              = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }
            if (scale < 0.5f) scale = 0.5f;
            if (scale > 3.0f) scale = 3.0f;
            style.ScaleAllSizes(scale);
            style.FontScaleMain = s_BaseStyle.FontScaleMain * scale;
            ImGui::GetStyle()   = style;
        }
    } // namespace

    void EditorContext::OpenProject(::Ref<EditorProject> project)
    {
        if (project) m_PendingProject = project;
    }

    void EditorContext::OpenProjectDialog()
    {
        if (auto path = Window::OpenFile(s_ProjectFilter, "."_pv))
            OpenProject(EditorProject::Load(*path));
    }

    bool EditorContext::OpenStartupProject()
    {
        const auto& name = Preferences.StartupProject;
        if (name.Empty()) return false;

        for (const auto& recent : Preferences.RecentProjects)
        {
            if (recent.Name != name) continue;
            VtCoreTraceTag("Editor", "Opening the startup project '{}'", name);
            auto project = EditorProject::Load(recent.Path);
            OpenProject(project);
            return project != nullptr;
        }
        VtCoreErrorTag("Editor",
                       "The startup project '{}' isn't among the recent "
                       "projects in {}",
                       name, kPreferencesPath);
        return false;
    }

    bool EditorContext::LoadPreferences()
    {
        bool loaded = DeserializeJsonFromFile(kPreferencesPath, Preferences);
        if (!loaded)
            VtCoreWarnTag("Editor", "Can't read {}; using defaults", kPreferencesPath);
        for (const auto& shortcut : Preferences.Shortcuts)
            Commands.SetShortcut(shortcut.Command, shortcut.Chord);
        ApplyPreferences();
        return loaded;
    }

    void EditorContext::StoreShortcuts()
    {
        Preferences.Shortcuts.Clear();
        for (const auto& command : Commands.Commands())
            if (command.IsRebound())
                Preferences.Shortcuts.PushBack({command.Id, command.Shortcut});
    }

    void EditorContext::ApplyPendingPreferences()
    {
        m_ApplyPending = false;
        ApplyAppearance(Preferences.Theme, Preferences.UIScale);

        auto swapChain = App.Window().SwapChain();
        if (swapChain && swapChain->VSync() != Preferences.VSync)
        {
            // The render thread presents with it: let it finish first.
            Renderer::WaitIdle();
            swapChain->SetVSync(Preferences.VSync);
        }
    }

    void EditorContext::UpdateAutoSave(f32 deltaTime)
    {
        if (!Preferences.AutoSave || Scenes.IsRunning() || Scenes.ScenePath().Empty())
        {
            m_AutoSaveTimer = 0.0f;
            return;
        }
        m_AutoSaveTimer += deltaTime;
        if (m_AutoSaveTimer < Preferences.AutoSaveMinutes * 60.0f) return;
        m_AutoSaveTimer = 0.0f;
        Scenes.SaveScene();
    }

    void EditorContext::SavePreferences() const
    {
        if (!SerializeJsonToFile(kPreferencesPath, Preferences))
            VtCoreErrorTag("Editor", "Can't write {}", kPreferencesPath);
    }

    void EditorContext::Update(f32 deltaTime)
    {
        VtProfileFunction();
        Scripts.Update(deltaTime, Preferences.AutoCompileScripts, Scenes.IsRunning());
        if (m_ApplyPending) ApplyPendingPreferences();
        if (m_PendingProject)
        {
            // Projects remount the filesystem: nothing may be loading.
            Renderer::WaitIdle();
            EditorProject::SwitchTo(Move(m_PendingProject));
            m_PendingProject = nullptr;
        }
        Scenes.Update(deltaTime);
        UpdateAutoSave(deltaTime);
        Windows.Update(*this, deltaTime);
    }

    void EditorContext::OnProjectChanged(const ProjectChangedEvent& event)
    {
        auto& project = *event.Project;
        auto& config  = project.RuntimeProject()->Config();

        Preferences.InsertRecentProject(
            {config.Metadata.Name, Path(project.ConfigPath()), event.Time});
        SavePreferences();

        // A project without a start scene (or with a stale path) keeps the
        // scene that's open.
        Path startScene = Path(project.Path()) / config.StartScenePath;
        if (config.StartScenePath.Empty())
            VtCoreWarnTag("Editor", "The project '{}' has no start scene",
                          config.Metadata.Name);
        else if (!Filesystem::Exists(startScene))
            VtCoreWarnTag("Editor", "The start scene '{}' of the project '{}' doesn't exist",
                          startScene, config.Metadata.Name);
        else Scenes.OpenScene(startScene);
        Windows.NotifyProjectChanged(*this, project);
    }
} // namespace Vortex::Editor
