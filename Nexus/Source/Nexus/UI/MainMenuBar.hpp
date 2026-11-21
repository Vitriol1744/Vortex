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

    /// The main menu bar, built from what is registered: each menu lists
    /// its commands (by group), then the toggles of its windows.
    void DrawMainMenuBar(EditorContext& context);
} // namespace Vortex::Editor
