/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorContext.hpp>
#include <Nexus/UI/MainMenuBar.hpp>

#include <imgui.h>

import Prism.Containers.Vector;

namespace Vortex::Editor
{
    namespace
    {
        /// Menus in this order first; any other menu a command or window
        /// names follows, in registration order.
        constexpr StringView kMenuOrder[]
            = {"File", "Edit", "Scene", "View", "Tools"};

        void AddMenu(Vector<String>& menus, StringView menu)
        {
            if (menu.Empty()) return;
            for (const auto& existing : menus)
                if (existing == menu) return;
            menus.PushBack(String(menu));
        }

        Vector<String> CollectMenus(const EditorContext& context)
        {
            Vector<String> menus;
            for (StringView menu : kMenuOrder) AddMenu(menus, menu);
            for (const auto& command : context.Commands.Commands())
                AddMenu(menus, command.Menu);
            for (const auto& window : context.Windows.Windows())
                AddMenu(menus, window->Menu());
            return menus;
        }

        bool HasCommands(const EditorContext& context, StringView menu)
        {
            for (const auto& command : context.Commands.Commands())
                if (command.Menu == menu) return true;
            return false;
        }
    } // namespace

    void DrawMainMenuBar(EditorContext& context)
    {
        if (!ImGui::BeginMainMenuBar()) return;
        for (const auto& menu : CollectMenus(context))
        {
            bool hasCommands = HasCommands(context, menu);
            bool hasWindows  = context.Windows.HasMenuItems(menu);
            if (!hasCommands && !hasWindows) continue;
            if (!ImGui::BeginMenu(menu.Raw())) continue;

            context.Commands.DrawMenuItems(menu);
            if (hasCommands && hasWindows) ImGui::Separator();
            context.Windows.DrawMenuItems(menu);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
} // namespace Vortex::Editor
