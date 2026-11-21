/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <imgui.h>
#include <imgui_internal.h>

module Nexus.Commands;

namespace Vortex::Editor
{
    EditorCommand& CommandRegistry::Register(EditorCommand command)
    {
        command.DefaultShortcut = command.Shortcut;
        if (EditorCommand* existing = Find(command.Id))
        {
            // Re-registering keeps a binding the user changed.
            int shortcut = existing->IsRebound() ? existing->Shortcut : command.Shortcut;
            *existing    = Prism::Move(command);
            existing->Shortcut = shortcut;
            return *existing;
        }
        m_Commands.PushBack(Prism::Move(command));
        return m_Commands.Back();
    }

    EditorCommand* CommandRegistry::Find(StringView id)
    {
        for (auto& command : m_Commands)
            if (command.Id == id) return &command;
        return nullptr;
    }

    bool CommandRegistry::Execute(StringView id)
    {
        EditorCommand* command = Find(id);
        if (!command || !command->Execute || !command->Enabled()) return false;
        command->Execute();
        return true;
    }

    void CommandRegistry::ProcessShortcuts()
    {
        if (!m_ShortcutsEnabled) return;
        // Collected first: a command may register or replace commands.
        Vector<String> fired;
        for (const auto& command : m_Commands)
        {
            if (!command.Shortcut) continue;
            // Global route: fires unless the focused window or an active
            // widget (e.g. a text field) claims the same keys.
            if (ImGui::Shortcut(command.Shortcut, ImGuiInputFlags_RouteGlobal))
                fired.PushBack(command.Id);
        }
        for (const auto& id : fired) Execute(id);
    }

    bool CommandRegistry::MenuItem(StringView id)
    {
        EditorCommand* command = Find(id);
        if (!command) return false;

        String shortcut = ShortcutText(command->Shortcut);
        bool   checked  = command->IsChecked && command->IsChecked();
        if (!ImGui::MenuItem(command->Label.Raw(),
                             shortcut.Empty() ? nullptr : shortcut.Raw(),
                             checked, command->Enabled()))
            return false;
        // The command may replace itself: copy what's needed first.
        String commandId = command->Id;
        return Execute(commandId);
    }

    bool CommandRegistry::DrawMenuItems(StringView menu)
    {
        // Ids first: running a command may change the registry.
        Vector<String> ids;
        Vector<int>    groups;
        for (const auto& command : m_Commands)
        {
            if (command.Menu != menu) continue;
            ids.PushBack(command.Id);
            groups.PushBack(command.Group);
        }
        for (decltype(ids.Size()) i = 0; i < ids.Size(); ++i)
        {
            if (i > 0 && groups[i] != groups[i - 1]) ImGui::Separator();
            MenuItem(ids[i]);
        }
        return !ids.Empty();
    }

    String CommandRegistry::ShortcutText(int chord)
    {
        if (!chord) return String{};
        // ImGui formats chords ("Ctrl+Shift+S") itself.
        return String(ImGui::GetKeyChordName(chord));
    }

    bool CommandRegistry::SetShortcut(StringView id, int chord)
    {
        EditorCommand* command = Find(id);
        if (!command) return false;
        command->Shortcut = chord;
        return true;
    }

    void CommandRegistry::ResetShortcut(StringView id)
    {
        if (EditorCommand* command = Find(id)) command->Shortcut = command->DefaultShortcut;
    }

    void CommandRegistry::ResetAllShortcuts()
    {
        for (auto& command : m_Commands) command.Shortcut = command.DefaultShortcut;
    }

    EditorCommand* CommandRegistry::FindByShortcut(int chord, StringView except)
    {
        if (!chord) return nullptr;
        for (auto& command : m_Commands)
            if (command.Shortcut == chord && command.Id != except) return &command;
        return nullptr;
    }
} // namespace Vortex::Editor
