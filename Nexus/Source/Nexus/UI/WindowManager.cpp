/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <cstdlib>
#include <cstring>
#include <imgui.h>
#include <imgui_internal.h>

module Nexus.EditorWindow;

namespace Vortex::Editor
{
    namespace
    {
        constexpr const char* kSettingsType = "NexusWindow";
    }

    WindowManager::~WindowManager()
    {
        // The handler points at this manager.
        if (m_HandlerRegistered && ImGui::GetCurrentContext())
            ImGui::RemoveSettingsHandler(kSettingsType);
    }

    EditorWindow* WindowManager::Find(StringView title) const
    {
        for (const auto& window : m_Windows)
            if (window->Title() == title) return window.Raw();
        return nullptr;
    }

    void WindowManager::Update(EditorContext& context, float deltaTime)
    {
        for (const auto& window : m_Windows) window->OnUpdate(context, deltaTime);
    }

    void WindowManager::Render(EditorContext& context)
    {
        for (const auto& window : m_Windows)
            if (window->Open) window->OnRender(context);
    }

    void WindowManager::Draw(EditorContext& context)
    {
        for (const auto& window : m_Windows)
        {
            if (!window->Open) continue;
            if (window->SelfDrawn)
            {
                window->OnDraw(context);
                if (!window->Open) ImGui::MarkIniSettingsDirty();
                continue;
            }

            window->OnBeforeBegin(context);
            bool visible = ImGui::Begin(window->Title().Raw(),
                                        window->Closable ? &window->Open : nullptr,
                                        window->Flags);
            window->OnAfterBegin(context);
            // Collapsed or fully clipped windows skip their contents.
            if (visible) window->OnDraw(context);
            ImGui::End();

            if (!window->Open) ImGui::MarkIniSettingsDirty();
        }
    }

    void WindowManager::NotifyProjectChanged(EditorContext& context,
                                             EditorProject& project)
    {
        for (const auto& window : m_Windows)
            window->OnProjectChanged(context, project);
    }

    bool WindowManager::DrawMenuItems(StringView menu)
    {
        bool any = false;
        for (const auto& window : m_Windows)
        {
            if (window->Menu() != menu) continue;
            any = true;
            if (ImGui::MenuItem(window->Title().Raw(), nullptr, &window->Open))
                ImGui::MarkIniSettingsDirty();
        }
        return any;
    }

    bool WindowManager::HasMenuItems(StringView menu) const
    {
        for (const auto& window : m_Windows)
            if (window->Menu() == menu) return true;
        return false;
    }

    void WindowManager::RegisterSettingsHandler()
    {
        if (m_HandlerRegistered) return;
        m_HandlerRegistered = true;

        // [NexusWindow][State]
        // Console=1
        ImGuiSettingsHandler handler{};
        handler.TypeName   = kSettingsType;
        handler.TypeHash   = ImHashStr(kSettingsType);
        handler.UserData   = this;
        handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*,
                                const char* name) -> void*
        { return std::strcmp(name, "State") == 0 ? reinterpret_cast<void*>(1) : nullptr; };
        handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler* handler,
                                void*, const char* line)
        {
            // Titles may contain '=' only before the last one.
            const char* separator = std::strrchr(line, '=');
            if (!separator) return;
            auto*      self = static_cast<WindowManager*>(handler->UserData);
            StringView title(line, static_cast<decltype(sizeof 0)>(separator - line));
            if (EditorWindow* window = self->Find(title))
                window->Open = std::atoi(separator + 1) != 0;
        };
        handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler,
                                ImGuiTextBuffer* buffer)
        {
            auto* self = static_cast<WindowManager*>(handler->UserData);
            buffer->appendf("[%s][State]\n", handler->TypeName);
            for (const auto& window : self->m_Windows)
                buffer->appendf("%s=%d\n", window->Title().Raw(), window->Open ? 1 : 0);
            buffer->append("\n");
        };
        ImGui::AddSettingsHandler(&handler);
    }
} // namespace Vortex::Editor
