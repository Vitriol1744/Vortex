/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Vortex::Editor
{
    class EditorContext;

    /// The editor's built-in actions (scenes, projects, play mode, tools).
    /// Each lists itself in a menu and/or binds a shortcut, so adding an
    /// action is one Register call here.
    void RegisterEditorCommands(EditorContext& context);
} // namespace Vortex::Editor
