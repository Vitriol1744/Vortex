/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorContext.hpp>

namespace Vortex::Editor
{
    /// Frame timing, GPU memory and the viewport camera's projection.
    class StatisticsWindow final : public EditorWindow
    {
      public:
        StatisticsWindow()
            : EditorWindow("Statistics")
        {
        }
        void OnDraw(EditorContext& context) override;
    };

    /// Dear ImGui's style editor, to tune the editor theme.
    class StyleEditorWindow final : public EditorWindow
    {
      public:
        StyleEditorWindow()
            : EditorWindow("Style Editor", "Tools")
        {
            Open = false;
        }
        void OnDraw(EditorContext& context) override;
    };

    /// Dear ImGui's demo: every widget, for reference.
    class ImGuiDemoWindow final : public EditorWindow
    {
      public:
        ImGuiDemoWindow()
            : EditorWindow("ImGui Demo", "Tools")
        {
            Open      = false;
            SelfDrawn = true;
        }
        void OnDraw(EditorContext& context) override;
    };
} // namespace Vortex::Editor
