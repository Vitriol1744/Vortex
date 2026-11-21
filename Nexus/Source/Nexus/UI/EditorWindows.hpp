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

    /// Every window of the editor, in menu order. A new window is one line
    /// here (see EditorWindow).
    void RegisterEditorWindows(EditorContext& context);
} // namespace Vortex::Editor
