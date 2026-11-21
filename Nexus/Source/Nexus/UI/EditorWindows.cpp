/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/UI/EditorWindows.hpp>
#include <Nexus/UI/Windows/ConsoleWindow.hpp>
#include <Nexus/UI/Windows/ContentBrowserWindow.hpp>
#include <Nexus/UI/Windows/SceneHierarchyWindow.hpp>
#include <Nexus/UI/Windows/SceneViewportWindow.hpp>
#include <Nexus/UI/Windows/ToolWindows.hpp>

namespace Vortex::Editor
{
    void RegisterEditorWindows(EditorContext& context)
    {
        auto& windows = context.Windows;
        // View
        windows.Add<SceneViewportWindow>(context);
        windows.Add<SceneHierarchyWindow>(context);
        windows.Add<InspectorWindow>(context);
        windows.Add<ContentBrowserWindow>(context);
        windows.Add<ConsoleWindow>(context);
        windows.Add<StatisticsWindow>(context);
        // Tools
        windows.Add<StyleEditorWindow>(context);
        windows.Add<ImGuiDemoWindow>(context);

        // Open states are restored from imgui.ini on the first frame.
        windows.RegisterSettingsHandler();
    }
} // namespace Vortex::Editor
