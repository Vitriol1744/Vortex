/*
 * Created by v1tr10l7 on 15.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorCamera.hpp>
#include <Nexus/EditorProject.hpp>

import Prism.Containers.Vector;
import Prism.String.String;
import Prism.Utility.Path;

namespace Vortex::Editor
{
    struct RecentProject
    {
        String Name;
        Path   Path;
        time_t LastOpened;

        void   Serialize(ArchiveWriter& archive) const;
        bool   Deserialize(ArchiveReader& archive);
    };

    /// Projection and overlays of the scene viewport.
    struct ViewportSettings
    {
        f32  FieldOfView = 75.0f; // degrees
        f32  Near        = 0.1f;
        f32  Far         = 100.0f;
        /// The ground grid (XZ plane) under the scene.
        bool ShowGrid    = true;
    };

    enum class EditorTheme : i32
    {
        eVortexDark,
        eImGuiDark,
        eLight,
        eClassic,
    };

    /// A shortcut the user changed; commands not listed keep their default.
    struct ShortcutOverride
    {
        String Command{};
        /// ImGuiKeyChord; 0 = unbound.
        i32    Chord = 0;
    };

    struct EditorPreferences
    {
        static constexpr usize MaxRecentProjects = 10;
        /// Next to the working directory, like imgui.ini.
        static constexpr const char* FileName = "editorPreferences.json";

        Path                   StartupProject;
        /// Newest first, one entry per project file.
        Vector<RecentProject>  RecentProjects;

        // General ----------------------------------------------------------
        bool                   OpenLastProject = true;
        bool                   AutoSave        = false;
        f32                    AutoSaveMinutes = 5.0f;
        /// Rebuild the C# scripts when their sources change.
        bool                   AutoCompileScripts = true;

        // Appearance -------------------------------------------------------
        EditorTheme            Theme   = EditorTheme::eVortexDark;
        f32                    UIScale = 1.0f;

        // Viewport and input -------------------------------------------------
        ViewportSettings       Viewport;
        EditorCameraControls   Camera;
        Vector<ShortcutOverride> Shortcuts;

        // Graphics (read before the renderer starts) -------------------------
        /// RHI::AdapterInfo::Name; empty = pick the best GPU.
        String                 PreferredGpu{};
        /// Graphics API by name (RHI::APIName); applied at startup.
        String                 RendererAPI{"Vulkan"};
        bool                   VSync = false;

        void InsertRecentProject(const RecentProject& recent);

        void Serialize(ArchiveWriter& archive) const;
        bool Deserialize(ArchiveReader& archive);
    };
}; // namespace Vortex::Editor
