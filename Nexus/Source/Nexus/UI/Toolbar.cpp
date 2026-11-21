/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorContext.hpp>
#include <Nexus/EditorResources.hpp>
#include <Nexus/UI/Toolbar.hpp>

#include <Vortex/Renderer/ImGui/ImGui.hpp>

#include <imgui.h>

namespace Vortex::Editor
{
    namespace
    {
        constexpr f32 kHeight     = 32.0f;
        constexpr f32 kButtonSize = kHeight - 8.0f;
    } // namespace

    Toolbar::Toolbar()
        : m_Play(Resources::LoadTexture("assets/icons/play.png"))
        , m_Simulate(Resources::LoadTexture("assets/icons/simulate.png"))
        , m_Stop(Resources::LoadTexture("assets/icons/stop.png"))
        , m_Pause(Resources::LoadTexture("assets/icons/pause.png"))
        , m_Step(Resources::LoadTexture("assets/icons/step.png"))
    {
    }

    void Toolbar::Button(EditorContext& context, const ::Ref<TextureAsset>& icon,
                         StringView command, bool highlighted)
    {
        EditorCommand* entry   = context.Commands.Find(command);
        bool           enabled = entry && entry->Enabled();

        ImVec4 background = highlighted
                              ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive)
                              : ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        ImVec4 tint(1.0f, 1.0f, 1.0f, enabled ? 1.0f : 0.4f);

        ImGui::BeginDisabled(!enabled);
        if (ImGui::ImageButton(command.Raw(), UI::TextureID(icon->Texture()),
                               ImVec2(kButtonSize, kButtonSize), ImVec2(0, 0),
                               ImVec2(1, 1), background, tint))
            context.Commands.Execute(command);
        ImGui::EndDisabled();

        if (entry && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            String shortcut = CommandRegistry::ShortcutText(entry->Shortcut);
            if (shortcut.Empty()) ImGui::SetTooltip("%s", entry->Label.Raw());
            else ImGui::SetTooltip("%s (%s)", entry->Label.Raw(), shortcut.Raw());
        }
    }

    void Toolbar::Draw(EditorContext& context)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 4.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImVec4 hovered = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        hovered.w      = 0.5f;
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hovered);

        constexpr ImGuiWindowFlags flags
            = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if (ImGui::BeginChild("##Toolbar", ImVec2(0.0f, kHeight),
                              ImGuiChildFlags_None, flags))
        {
            const auto& scenes  = context.Scenes;
            bool        running = scenes.IsRunning();
            int         buttons = running ? 3 : 2;
            f32         spacing = ImGui::GetStyle().ItemSpacing.x;
            f32         width   = kButtonSize * buttons + spacing * (buttons - 1);
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - width) * 0.5f);

            if (!running)
            {
                Button(context, m_Play, "Scene.Play");
                ImGui::SameLine();
                Button(context, m_Simulate, "Scene.Simulate");
            }
            else {
                Button(context, m_Stop, "Scene.Stop");
                ImGui::SameLine();
                Button(context, m_Pause, "Scene.Pause", scenes.IsPaused());
                ImGui::SameLine();
                Button(context, m_Step, "Scene.Step");
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }
} // namespace Vortex::Editor
