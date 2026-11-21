/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
export module Nexus.Commands;

import Prism.Containers.Vector;
import Prism.Core.TypeTraits;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Utility.Delegate;

export namespace Vortex::Editor
{
    using Prism::Delegate;
    using Prism::String;
    using Prism::StringView;
    using Prism::Vector;

    /**
     * An action of the editor. The menu bar, the toolbar and keyboard
     * shortcuts all run commands, so a feature is written once and is
     * reachable from everywhere (and undo/redo has a single entry point to
     * hook into later).
     */
    struct EditorCommand
    {
        /// Stable identifier, e.g. "Scene.Save".
        String                Id;
        /// Menu text, e.g. "Save Scene".
        String                Label;
        /// Top-level menu listing it ("File", "Scene"...); empty = none
        /// (shortcut or toolbar only).
        String                Menu;
        /// Separators go between groups of a menu.
        int                   Group    = 0;
        /// ImGuiKeyChord (ImGuiMod_* | ImGuiKey_*); 0 = no shortcut.
        int                   Shortcut = 0;
        /// The shortcut it was registered with (Shortcut may be rebound).
        int                   DefaultShortcut = 0;
        Delegate<void()>      Execute;
        /// Optional; a command without it is always enabled.
        Delegate<bool()>      CanExecute;
        /// Optional; shown as a check mark in menus (toggles).
        Delegate<bool()>      IsChecked;

        bool                  Enabled() const { return !CanExecute || CanExecute(); }
        bool                  IsRebound() const { return Shortcut != DefaultShortcut; }
    };

    class CommandRegistry
    {
      public:
        /// Replaces a command registered under the same id.
        EditorCommand&       Register(EditorCommand command);
        EditorCommand*       Find(StringView id);

        /// Runs the command if it exists and is enabled.
        bool                 Execute(StringView id);

        /// Fires commands whose shortcut was pressed this frame. Call once
        /// per ImGui frame, outside any window. Keys a focused text field
        /// owns aren't stolen from it.
        void                 ProcessShortcuts();

        /// A menu item for the command (label, shortcut text, enabled and
        /// check state); runs it when clicked. Inside BeginMenu/EndMenu.
        bool                 MenuItem(StringView id);
        /// The items of every command listed under `menu`, separated by
        /// group; false if there are none.
        bool                 DrawMenuItems(StringView menu);

        /// "Ctrl+Shift+S" for a chord.
        static String        ShortcutText(int chord);

        // Key bindings -----------------------------------------------------
        /// Rebinds `id` (0 unbinds). False if there's no such command.
        bool                 SetShortcut(StringView id, int chord);
        void                 ResetShortcut(StringView id);
        void                 ResetAllShortcuts();
        /// The command bound to `chord`, other than `except`.
        EditorCommand*       FindByShortcut(int chord, StringView except = {});
        /// Off while the settings capture a key, so it doesn't also fire.
        void                 SetShortcutsEnabled(bool enabled) { m_ShortcutsEnabled = enabled; }

        const Vector<EditorCommand>& Commands() const { return m_Commands; }

      private:
        // Registration order: stable menus and shortcut priority.
        Vector<EditorCommand> m_Commands;
        bool                  m_ShortcutsEnabled = true;
    };
} // namespace Vortex::Editor
