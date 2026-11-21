/*
 * Created by v1tr10l7 on 15.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorContext.hpp>

namespace Vortex::Editor
{
    /// The actors of the active scene; selects, adds and removes them.
    class SceneHierarchyWindow final : public EditorWindow
    {
      public:
        SceneHierarchyWindow()
            : EditorWindow("Scene Hierarchy")
        {
        }
        void OnDraw(EditorContext& context) override;
    };

    /// Components of the selected actor.
    class InspectorWindow final : public EditorWindow
    {
      public:
        InspectorWindow()
            : EditorWindow("Inspector")
        {
        }
        void OnDraw(EditorContext& context) override;
    };
}; // namespace Vortex::Editor
