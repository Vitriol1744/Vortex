/*
 * Created by v1tr10l7 on 15.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorPreferences.hpp>

import Prism.Algorithm.Sort;

namespace Vortex::Editor
{
    namespace
    {
        void WriteViewport(ArchiveWriter& archive, const ViewportSettings& viewport)
        {
            archive.BeginObject("Viewport");
            archive.Write("FieldOfView", viewport.FieldOfView);
            archive.Write("Near", viewport.Near);
            archive.Write("Far", viewport.Far);
            archive.Write("ShowGrid", viewport.ShowGrid);
            archive.EndObject();
        }
        void ReadViewport(ArchiveReader& archive, ViewportSettings& viewport)
        {
            if (!archive.BeginObject("Viewport")) return;
            archive.Read("FieldOfView", viewport.FieldOfView);
            archive.Read("Near", viewport.Near);
            archive.Read("Far", viewport.Far);
            archive.Read("ShowGrid", viewport.ShowGrid);
            archive.EndObject();
        }

        void WriteCamera(ArchiveWriter& archive, const EditorCameraControls& camera)
        {
            archive.BeginObject("Camera");
            archive.Write("Forward", camera.Forward);
            archive.Write("Backward", camera.Backward);
            archive.Write("Left", camera.Left);
            archive.Write("Right", camera.Right);
            archive.Write("Up", camera.Up);
            archive.Write("Down", camera.Down);
            archive.Write("Fast", camera.Fast);
            archive.Write("TurnLeft", camera.TurnLeft);
            archive.Write("TurnRight", camera.TurnRight);
            archive.Write("MoveSpeed", camera.MoveSpeed);
            archive.Write("FastMultiplier", camera.FastMultiplier);
            archive.Write("LookSensitivity", camera.LookSensitivity);
            archive.Write("InvertY", camera.InvertY);
            archive.EndObject();
        }
        void ReadCamera(ArchiveReader& archive, EditorCameraControls& camera)
        {
            if (!archive.BeginObject("Camera")) return;
            archive.Read("Forward", camera.Forward);
            archive.Read("Backward", camera.Backward);
            archive.Read("Left", camera.Left);
            archive.Read("Right", camera.Right);
            archive.Read("Up", camera.Up);
            archive.Read("Down", camera.Down);
            archive.Read("Fast", camera.Fast);
            archive.Read("TurnLeft", camera.TurnLeft);
            archive.Read("TurnRight", camera.TurnRight);
            archive.Read("MoveSpeed", camera.MoveSpeed);
            archive.Read("FastMultiplier", camera.FastMultiplier);
            archive.Read("LookSensitivity", camera.LookSensitivity);
            archive.Read("InvertY", camera.InvertY);
            archive.EndObject();
        }

        void SortRecent(Vector<RecentProject>& projects)
        {
            Prism::Sort(projects.begin(), projects.end(),
                 [](const RecentProject& a, const RecentProject& b)
                 { return a.LastOpened > b.LastOpened; });
        }
    } // namespace

    void RecentProject::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("Name", Name.View());
        archive.Write("Path", Path.View());
        archive.Write("LastOpened", LastOpened);
    }
    bool RecentProject::Deserialize(ArchiveReader& archive)
    {
        archive.Read("Name", Name);
        archive.Read("Path", Path);
        archive.Read("LastOpened", LastOpened);

        return true;
    }

    void EditorPreferences::InsertRecentProject(const RecentProject& recent)
    {
        // One entry per project file: reopening moves it to the front.
        for (usize i = 0; i < RecentProjects.Size();)
        {
            if (RecentProjects[i].Path == recent.Path)
                RecentProjects.Erase(RecentProjects.begin() + i);
            else ++i;
        }
        RecentProjects.Insert(RecentProjects.begin(), recent);
        while (RecentProjects.Size() > MaxRecentProjects) RecentProjects.PopBack();
        StartupProject = recent.Name;
    }

    void EditorPreferences::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("StartupProject", StartupProject.View());

        usize count = RecentProjects.Size() < MaxRecentProjects
                        ? RecentProjects.Size()
                        : MaxRecentProjects;
        archive.BeginArray("RecentProjects", count);
        for (usize i = 0; i < count; ++i)
            SerializeValue(archive, "", RecentProjects[i]);
        archive.EndArray();

        archive.BeginObject("Editor");
        archive.Write("OpenLastProject", OpenLastProject);
        archive.Write("AutoSave", AutoSave);
        archive.Write("AutoSaveMinutes", AutoSaveMinutes);
        archive.Write("AutoCompileScripts", AutoCompileScripts);
        archive.Write("Theme", static_cast<i32>(Theme));
        archive.Write("UIScale", UIScale);
        archive.EndObject();

        archive.BeginObject("Graphics");
        archive.Write("RendererAPI", RendererAPI.View());
        archive.Write("PreferredGpu", PreferredGpu.View());
        archive.Write("VSync", VSync);
        archive.EndObject();

        WriteViewport(archive, Viewport);
        WriteCamera(archive, Camera);

        archive.BeginArray("Shortcuts", Shortcuts.Size());
        for (const auto& shortcut : Shortcuts)
        {
            archive.BeginObject("");
            archive.Write("Command", shortcut.Command.View());
            archive.Write("Chord", shortcut.Chord);
            archive.EndObject();
        }
        archive.EndArray();
    }

    bool EditorPreferences::Deserialize(ArchiveReader& archive)
    {
        archive.Read("StartupProject", StartupProject);

        usize count = 0;
        if (archive.BeginArray("RecentProjects", count))
        {
            for (usize i = 0; i < count; ++i)
            {
                RecentProject recent;
                if (!DeserializeValue(archive, "", recent)) continue;
                bool duplicate = false;
                for (const auto& existing : RecentProjects)
                    duplicate = duplicate || existing.Path == recent.Path;
                if (!duplicate) RecentProjects.PushBack(recent);
            }
            archive.EndArray();
        }
        SortRecent(RecentProjects);

        if (archive.BeginObject("Editor"))
        {
            i32 theme = static_cast<i32>(Theme);
            archive.Read("OpenLastProject", OpenLastProject);
            archive.Read("AutoSave", AutoSave);
            archive.Read("AutoSaveMinutes", AutoSaveMinutes);
            archive.Read("AutoCompileScripts", AutoCompileScripts);
            archive.Read("Theme", theme);
            archive.Read("UIScale", UIScale);
            if (theme >= 0 && theme <= static_cast<i32>(EditorTheme::eClassic))
                Theme = static_cast<EditorTheme>(theme);
            archive.EndObject();
        }
        if (archive.BeginObject("Graphics"))
        {
            archive.Read("RendererAPI", RendererAPI);
            archive.Read("PreferredGpu", PreferredGpu);
            archive.Read("VSync", VSync);
            archive.EndObject();
        }
        ReadViewport(archive, Viewport);
        ReadCamera(archive, Camera);

        if (archive.BeginArray("Shortcuts", count))
        {
            for (usize i = 0; i < count; ++i)
            {
                if (!archive.BeginObject("")) continue;
                ShortcutOverride shortcut;
                archive.Read("Command", shortcut.Command);
                archive.Read("Chord", shortcut.Chord);
                archive.EndObject();
                if (!shortcut.Command.Empty()) Shortcuts.PushBack(shortcut);
            }
            archive.EndArray();
        }
        return true;
    }
}; // namespace Vortex::Editor
