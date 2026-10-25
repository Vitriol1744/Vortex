/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorProject.hpp>
#include <Nexus/UI/Panel.hpp>

#include <Prism/Containers/Array.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>

#include <imgui.h>

namespace Vortex::Editor
{
    enum class SettingsPage
    {
        eProjectSettings,
        eEditorSettings,
        eKeymaps
    };
    enum class SettingsCategory
    {
        eProject_Metadata,
        eProject_Renderer,
        eProject_Physics,
        eEditor_General,
        eEditor_Shortcuts
    };

    class SettingsOverlay
    {
      public:
        void Render(bool* pOpen)
        {
            if (!*pOpen) return;

            // -------------------------------------------------------------------
            // 1. COVER THE MAIN VIEWPORT
            // -------------------------------------------------------------------
            const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(mainViewport->WorkPos);
            ImGui::SetNextWindowSize(mainViewport->WorkSize);
            ImGui::SetNextWindowViewport(mainViewport->ID);

            ImGuiWindowFlags windowFlags
                = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings
                | ImGuiWindowFlags_NoDocking;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                ImVec2(20.0f, 20.0f));

            // Force focus to ensure it renders on top when opened
            ImGui::SetNextWindowFocus();

            if (ImGui::Begin("##SettingsOverlay", pOpen, windowFlags))
            {
                // Top Action Bar / Header
                DrawOverlayHeader(pOpen);
                ImGui::Separator();
                ImGui::Spacing();

                // ---------------------------------------------------------------
                // 2. MAIN SPLIT TABLE (SIDEBAR + PAGE VIEWPORT)
                // ---------------------------------------------------------------
                constexpr ImGuiTableFlags tableFlags
                    = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV;

                if (ImGui::BeginTable(
                        "SettingsOverlayTable", 2, tableFlags,
                        ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
                {
                    ImGui::TableSetupColumn(
                        "Sidebar", ImGuiTableColumnFlags_WidthFixed, 220.0f);
                    ImGui::TableSetupColumn("PageContent",
                                            ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableNextRow();

                    // --- LEFT SIDEBAR: ROOT CATEGORIES ---
                    ImGui::TableSetColumnIndex(0);
                    ImGui::BeginChild("SettingsSidebar", ImVec2(0, 0),
                                      ImGuiChildFlags_None);
                    DrawSidebarNavigation();
                    ImGui::EndChild();

                    // --- RIGHT CONTENT PANE: ENTIRE PAGE DISPLAY ---
                    ImGui::TableSetColumnIndex(1);
                    ImGui::BeginChild("SettingsPageViewport", ImVec2(0, 0),
                                      ImGuiChildFlags_None);

                    ImGui::Indent(15.0f);
                    DrawActivePage();
                    ImGui::Unindent(15.0f);

                    ImGui::EndChild();

                    ImGui::EndTable();
                }
            }
            ImGui::End();

            ImGui::PopStyleVar(3);
        }

      private:
        SettingsPage m_ActivePage = SettingsPage::eProjectSettings;

        void         DrawOverlayHeader(bool* pOpen)
        {
            ImGui::PushFont(
                ImGui::GetIO()
                    .Fonts->Fonts[0]); // Base font bold/large if configured
            ImGui::TextUnformatted("Settings");
            ImGui::PopFont();

            ImGui::SameLine(ImGui::GetWindowWidth() - 90.0f);
            if (ImGui::Button("Close (ESC)", ImVec2(70, 25))
                || ImGui::IsKeyPressed(ImGuiKey_Escape))
                *pOpen = false;
        }

        void DrawSidebarNavigation()
        {
            ImGui::Spacing();

            auto DrawCategoryButton
                = [this](const char* label, SettingsPage page)
            {
                bool isSelected = (m_ActivePage == page);
                if (isSelected)
                    ImGui::PushStyleColor(
                        ImGuiCol_Header,
                        ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));

                if (ImGui::Selectable(label, isSelected,
                                      ImGuiSelectableFlags_DontClosePopups,
                                      ImVec2(0, 30.0f)))
                    m_ActivePage = page;

                if (isSelected) ImGui::PopStyleColor();
            };

            if (EditorProject::Active())
                DrawCategoryButton("  Project Settings",
                                   SettingsPage::eProjectSettings);
            else if (m_ActivePage == SettingsPage::eProjectSettings)
                m_ActivePage = SettingsPage::eEditorSettings;
            DrawCategoryButton("  Editor Settings",
                               SettingsPage::eEditorSettings);
            DrawCategoryButton("  Keymaps & Controls", SettingsPage::eKeymaps);
        }

        void DrawActivePage()
        {
            switch (m_ActivePage)
            {
                case SettingsPage::eProjectSettings:
                    DrawProjectSettingsPage();
                    break;

                case SettingsPage::eEditorSettings:
                    DrawEditorSettingsPage();
                    break;

                case SettingsPage::eKeymaps:
                    ImGui::TextDisabled("Keybindings page...");
                    break;
            }
        }

        // -----------------------------------------------------------------------
        // PAGE 1: PROJECT SETTINGS (STACKED HEADINGS & SEPARATORS)
        // -----------------------------------------------------------------------
        void DrawProjectSettingsPage()
        {
            // === SECTION 1: METADATA ===
            DrawSectionHeader("PROJECT METADATA");

            auto& activeProject = *EditorProject::Active();
            auto& config        = activeProject.RuntimeProject()->Config();

            UI::InputText("Project Name", config.Metadata.Name);
            // UI::InputText("Engine Version", &config.Metadata.Version);
            UI::InputText("Asset Path", config.AssetDirectory);
            UI::InputText("Default Scene", config.StartScenePath);

            ImGui::Spacing();
            ImGui::Spacing();

            // === SECTION 2: RENDERER ===
            DrawSectionHeader("RENDERER");

            auto& rendererSettings = config.RendererSettings;
            ImGui::Checkbox("Enable V-Sync", &rendererSettings.VSync);

            const char* msaaItems[]
                = {"Off (1x)", "2x MSAA", "4x MSAA", "8x MSAA"};
            i32 msaaIndex = (rendererSettings.AntiAliasingSamples == 8) ? 3
                          : (rendererSettings.AntiAliasingSamples == 4) ? 2
                          : (rendererSettings.AntiAliasingSamples == 2) ? 1
                                                                        : 0;

            if (ImGui::Combo("MSAA Quality", &msaaIndex, msaaItems,
                             IM_ARRAYSIZE(msaaItems)))
            {
                i32 samples[]                        = {1, 2, 4, 8};
                rendererSettings.AntiAliasingSamples = samples[msaaIndex];
            }

            // ImGui::SliderFloat("Gamma Correction",
            //                    &rendererSettings.GammaCorrection, 1.0f, 3.0f,
            // "%.2f");
            // ImGui::ColorEdit4(
            //     "Clear Color",
            //     rendererSettings.ClearColor.data());

            ImGui::Spacing();
            ImGui::Spacing();

            // === SECTION 3: PHYSICS ===
            DrawSectionHeader("PHYSICS");
        }

        // -----------------------------------------------------------------------
        // PAGE 2: EDITOR SETTINGS
        // -----------------------------------------------------------------------
        void DrawEditorSettingsPage()
        {
            DrawSectionHeader("GENERAL EDITOR");
            static bool  autoSave         = true;
            static float autoSaveInterval = 5.0f;
            ImGui::Checkbox("Auto-save Scenes", &autoSave);
            ImGui::SliderFloat("Save Interval (mins)", &autoSaveInterval, 1.0f,
                               30.0f);

            ImGui::Spacing();
            ImGui::Spacing();

            DrawSectionHeader("VIEWPORT & CAMERA");
            static float cameraSens = 0.5f;
            static float fov        = 60.0f;
            ImGui::SliderFloat("Look Sensitivity", &cameraSens, 0.1f, 2.0f);
            ImGui::SliderFloat("Default FOV", &fov, 30.0f, 110.0f);
        }

        // Helper to format consistent visual headings across pages
        void DrawSectionHeader(const char* title)
        {
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                ImVec4(0.4f, 0.7f, 1.0f, 1.0f)); // Subtle highlight color
            ImGui::TextUnformatted(title);
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();
        }
    };
} // namespace Vortex::Editor
