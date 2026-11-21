/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorContext.hpp>
#include <Vortex/Core/Core.hpp>
#include <Nexus/UI/Overlays/SettingsOverlay.hpp>

#include <Vortex/Renderer/ImGui/ImGui.hpp>

#include <imgui.h>

#include <cmath>
#include <cstdio>

import Prism.Containers.Vector;
import Prism.String.Formatter;
import Prism.String.StringView;
import Vortex.Renderer;

namespace Vortex::Editor
{
    namespace
    {
        constexpr ImVec4 kAccent     = {0.153f, 0.725f, 0.949f, 1.00f};
        constexpr ImVec4 kAccentSoft = {0.153f, 0.725f, 0.949f, 0.18f};
        constexpr ImVec4 kGood       = {0.45f, 0.85f, 0.52f, 1.00f};
        constexpr ImVec4 kWarning    = {1.00f, 0.76f, 0.30f, 1.00f};
        constexpr ImVec4 kBad        = {1.00f, 0.45f, 0.45f, 1.00f};

        constexpr ImGuiWindowFlags kModalFlags
            = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        constexpr const char* kPopupName = "##NexusSettings";

        ImVec4 Mix(const ImVec4& a, const ImVec4& b, f32 t)
        {
            return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t,
                    a.w + (b.w - a.w) * t};
        }

        // Layout helpers --------------------------------------------------

        void PushFontScale(f32 scale)
        {
            ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * scale);
        }

        void HelpMarker(const char* help)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::BeginItemTooltip())
            {
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 28.0f);
                ImGui::TextUnformatted(help);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }

        void PageTitle(const char* title, const char* description)
        {
            PushFontScale(1.45f);
            ImGui::TextUnformatted(title);
            ImGui::PopFont();
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            ImGui::TextWrapped("%s", description);
            ImGui::PopStyleColor();
            ImGui::Dummy({0.0f, 6.0f});
        }

        /// A rounded panel grouping related settings.
        void BeginCard(const char* id, const char* title)
        {
            const ImVec4 frame = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            const ImVec4 base  = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, Mix(base, frame, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_Border, Mix(frame, kAccent, 0.12f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {16.0f, 12.0f});
            ImGui::BeginChild(id, {0.0f, 0.0f},
                              ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders
                                  | ImGuiChildFlags_AlwaysUseWindowPadding);
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::PushStyleColor(ImGuiCol_Text, kAccent);
            ImGui::TextUnformatted(title);
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }
        void EndCard()
        {
            ImGui::EndChild();
            ImGui::Dummy({0.0f, 4.0f});
        }

        /// Label | widget rows inside a card.
        bool BeginRows(const char* id)
        {
            if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp)) return false;
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.42f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.58f);
            return true;
        }
        void EndRows() { ImGui::EndTable(); }

        /// Starts a row: the label, then the cursor in the value column with
        /// the item width filling it.
        void Row(const char* label, const char* help = nullptr)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);
            if (help) HelpMarker(help);
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
        }
        void ValueRow(const char* label, const char* value, ImVec4 color = {0, 0, 0, 0})
        {
            Row(label);
            if (color.w > 0.0f) ImGui::TextColored(color, "%s", value);
            else ImGui::TextUnformatted(value);
        }

        void FormatBytes(char* out, usize size, u64 bytes)
        {
            constexpr f64 kKiB = 1024.0;
            f64           value = static_cast<f64>(bytes);
            if (value >= kKiB * kKiB * kKiB)
                std::snprintf(out, size, "%.2f GiB", value / (kKiB * kKiB * kKiB));
            else if (value >= kKiB * kKiB)
                std::snprintf(out, size, "%.0f MiB", value / (kKiB * kKiB));
            else std::snprintf(out, size, "%.0f KiB", value / kKiB);
        }

        char Lower(char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c; }
        bool ContainsIgnoreCase(StringView text, StringView needle)
        {
            if (needle.Empty()) return true;
            if (needle.Size() > text.Size()) return false;
            for (usize i = 0; i + needle.Size() <= text.Size(); ++i)
            {
                usize j = 0;
                while (j < needle.Size() && Lower(text[i + j]) == Lower(needle[j])) ++j;
                if (j == needle.Size()) return true;
            }
            return false;
        }

        /// A small pill (e.g. "IN USE"), on the current line.
        void Badge(const char* text, ImVec4 color)
        {
            ImVec2 size = ImGui::CalcTextSize(text);
            // Next to the previous item if it fits, else on its own line.
            ImGui::SameLine();
            if (ImGui::GetContentRegionAvail().x < size.x + 16.0f) ImGui::NewLine();
            ImVec2      pos  = ImGui::GetCursorScreenPos();
            const auto& style = ImGui::GetStyle();
            pos.y += style.FramePadding.y * 0.5f;
            ImVec2 padding{6.0f, 1.0f};
            ImVec4 fill = color;
            fill.w      = 0.20f;
            ImGui::GetWindowDrawList()->AddRectFilled(
                pos, {pos.x + size.x + padding.x * 2, pos.y + size.y + padding.y * 2},
                ImGui::GetColorU32(fill), 6.0f);
            ImGui::SetCursorScreenPos({pos.x + padding.x, pos.y + padding.y});
            ImGui::TextColored(color, "%s", text);
            ImGui::SameLine(0.0f, padding.x);
            ImGui::NewLine();
        }

        bool IsModifierKey(ImGuiKey key)
        {
            return key >= ImGuiKey_LeftCtrl && key <= ImGuiKey_RightSuper;
        }

        const char* ThemeName(EditorTheme theme)
        {
            switch (theme)
            {
                case EditorTheme::eVortexDark: return "Vortex Dark";
                case EditorTheme::eImGuiDark: return "Dear ImGui Dark";
                case EditorTheme::eLight: return "Light";
                case EditorTheme::eClassic: return "Classic";
            }
            return "Vortex Dark";
        }

        /// `scene` relative to `directory` if it's inside it.
        Path RelativeTo(PathView scene, PathView directory)
        {
            StringView sceneText(scene.Raw(), scene.Size());
            StringView dirText(directory.Raw(), directory.Size());
            if (!dirText.Empty() && sceneText.StartsWith(dirText)
                && sceneText.Size() > dirText.Size() && sceneText[dirText.Size()] == '/')
                return Path(sceneText.Substr(dirText.Size() + 1));
            return Path(sceneText);
        }
    } // namespace

    //------------------------------------------------------------------------
    // Frame -------------------------------------------------------------------
    //------------------------------------------------------------------------

    void SettingsOverlay::Render(EditorContext& context)
    {
        if (!m_HaveStartupGpu)
        {
            m_StartupGpu     = context.Preferences.PreferredGpu;
            m_StartupAPI     = context.Preferences.RendererAPI;
            m_HaveStartupGpu = true;
        }
        if (context.ShowSettings && !m_WasOpen) OnOpen(context);
        if (!context.ShowSettings && m_WasOpen) OnClose(context);
        if (!context.ShowSettings) return;
        RecordFrameTime();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2               size{viewport->WorkSize.x * 0.82f, viewport->WorkSize.y * 0.84f};
        if (size.x < 760.0f) size.x = viewport->WorkSize.x;
        if (size.y < 520.0f) size.y = viewport->WorkSize.y;
        ImGui::SetNextWindowPos({viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
                                 viewport->WorkPos.y + viewport->WorkSize.y * 0.5f},
                                ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, {0.0f, 0.0f, 0.0f, 0.55f});
        bool open = ImGui::BeginPopupModal(kPopupName, nullptr, kModalFlags);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        if (!open)
        {
            // Closed from outside ImGui's popup stack (shouldn't happen).
            context.ShowSettings = false;
            return;
        }

        UpdateCapture(context);
        if (m_Capture == Capture::eNone) context.Commands.SetShortcutsEnabled(true);

        DrawHeader(context);

        const f32 footerHeight = ImGui::GetFrameHeightWithSpacing() + 14.0f;
        const f32 bodyHeight   = ImGui::GetContentRegionAvail().y - footerHeight;
        DrawSidebar(context);
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {24.0f, 16.0f});
        ImGui::BeginChild("##SettingsPage", {0.0f, bodyHeight},
                          ImGuiChildFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleVar();
        DrawPage(context);
        ImGui::EndChild();

        DrawFooter(context);

        // Escape closes, unless it just ended a key capture.
        if (m_Capture == Capture::eNone && m_CaptureEndedOn != ImGui::GetFrameCount()
            && ImGui::IsKeyPressed(ImGuiKey_Escape, false) && !ImGui::IsAnyItemActive())
            context.ShowSettings = false;
        if (!context.ShowSettings) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();

        if (!context.ShowSettings) OnClose(context);
    }

    void SettingsOverlay::OnOpen(EditorContext& context)
    {
        m_WasOpen = true;
        if (m_Page <= Page::eProjectRenderer && !EditorProject::Active())
            m_Page = Page::eGraphics;
        ImGui::OpenPopup(kPopupName);
        IgnoreUnused(context);
    }

    void SettingsOverlay::OnClose(EditorContext& context)
    {
        if (!m_WasOpen) return;
        m_WasOpen = false;
        EndCapture();
        context.Commands.SetShortcutsEnabled(true);
        context.StoreShortcuts();
        context.SavePreferences();
    }

    void SettingsOverlay::RecordFrameTime()
    {
        m_FrameTimes[m_FrameOffset] = ImGui::GetIO().DeltaTime * 1000.0f;
        m_FrameOffset               = (m_FrameOffset + 1) % kFrameHistory;
    }

    void SettingsOverlay::SetStatus(String status)
    {
        m_Status     = Move(status);
        m_StatusTime = ImGui::GetTime();
    }

    //------------------------------------------------------------------------
    // Chrome ------------------------------------------------------------------
    //------------------------------------------------------------------------

    void SettingsOverlay::DrawHeader(EditorContext& context)
    {
        ImDrawList* draw   = ImGui::GetWindowDrawList();
        ImVec2      origin = ImGui::GetCursorScreenPos();
        f32         width  = ImGui::GetWindowWidth();
        f32         height = 64.0f;
        draw->AddRectFilledMultiColor(origin, {origin.x + width, origin.y + height},
                                      ImGui::GetColorU32(kAccentSoft),
                                      ImGui::GetColorU32({0, 0, 0, 0}),
                                      ImGui::GetColorU32({0, 0, 0, 0}),
                                      ImGui::GetColorU32(kAccentSoft));
        draw->AddLine({origin.x, origin.y + height}, {origin.x + width, origin.y + height},
                      ImGui::GetColorU32(ImGuiCol_Separator));

        ImGui::SetCursorScreenPos({origin.x + 24.0f, origin.y + 10.0f});
        ImGui::BeginGroup();
        PushFontScale(1.6f);
        ImGui::TextUnformatted("Settings");
        ImGui::PopFont();
        ImGui::TextDisabled("Nexus  |  %s %s", g_EngineName.Raw(),
                            String(g_Version).Raw());
        ImGui::EndGroup();

        const char* close = "Close  (Esc)";
        f32         buttonWidth
            = ImGui::CalcTextSize(close).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetCursorScreenPos({origin.x + width - buttonWidth - 20.0f,
                                   origin.y + (height - ImGui::GetFrameHeight()) * 0.5f});
        if (ImGui::Button(close)) context.ShowSettings = false;

        ImGui::SetCursorScreenPos({origin.x, origin.y + height + 1.0f});
    }

    void SettingsOverlay::DrawSidebar(EditorContext& context)
    {
        const f32    footerHeight = ImGui::GetFrameHeightWithSpacing() + 14.0f;
        const ImVec4 base         = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Mix(base, {0, 0, 0, 1}, 0.25f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {12.0f, 14.0f});
        ImGui::BeginChild("##SettingsSidebar",
                          {230.0f, ImGui::GetContentRegionAvail().y - footerHeight},
                          ImGuiChildFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        auto section = [](const char* title)
        {
            ImGui::Dummy({0.0f, 4.0f});
            PushFontScale(0.82f);
            ImGui::TextDisabled("%s", title);
            ImGui::PopFont();
        };
        auto item = [this](const char* label, Page page)
        {
            bool selected = m_Page == page;
            ImGui::PushStyleColor(ImGuiCol_Header, kAccentSoft);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Mix(kAccentSoft, kAccent, 0.15f));
            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.0f, 0.5f});
            ImGui::PushID(static_cast<int>(page));
            if (ImGui::Selectable("##Nav", selected, 0, {0.0f, 30.0f})) m_Page = page;
            ImGui::PopID();
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            if (selected)
                ImGui::GetWindowDrawList()->AddRectFilled(
                    {min.x, min.y + 5.0f}, {min.x + 3.0f, max.y - 5.0f},
                    ImGui::GetColorU32(kAccent), 2.0f);
            ImVec2 text = ImGui::CalcTextSize(label);
            ImGui::GetWindowDrawList()->AddText(
                {min.x + 14.0f, min.y + (max.y - min.y - text.y) * 0.5f},
                ImGui::GetColorU32(selected ? kAccent : ImGui::GetStyleColorVec4(ImGuiCol_Text)),
                label);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        };

        if (EditorProject::Active())
        {
            section("PROJECT");
            item("General", Page::eProjectGeneral);
            item("Renderer", Page::eProjectRenderer);
        }
        section("EDITOR");
        item("General", Page::eEditorGeneral);
        item("Appearance", Page::eAppearance);
        item("Viewport", Page::eViewport);
        section("GRAPHICS");
        item("GPU & Display", Page::eGraphics);
        section("INPUT");
        item("Keyboard Shortcuts", Page::eShortcuts);
        item("Camera Controls", Page::eCameraControls);

        IgnoreUnused(context);
        ImGui::EndChild();
    }

    bool SettingsOverlay::NeedsRestart(const EditorContext& context) const
    {
        return m_StartupGpu != context.Preferences.PreferredGpu
            || m_StartupAPI != context.Preferences.RendererAPI;
    }

    void SettingsOverlay::DrawFooter(EditorContext& context)
    {
        ImGui::Separator();
        ImGui::SetCursorPos({24.0f, ImGui::GetCursorPosY() + 6.0f});
        f64 age = ImGui::GetTime() - m_StatusTime;
        if (!m_Status.Empty() && age < 4.0)
        {
            ImVec4 color = kAccent;
            color.w      = age > 3.0 ? static_cast<f32>(4.0 - age) : 1.0f;
            ImGui::TextColored(color, "%s", m_Status.Raw());
        }
        else ImGui::TextDisabled("Editor settings are saved when this window closes.");

        if (NeedsRestart(context))
        {
            const char* label = "Restart to apply";
            f32         width = ImGui::CalcTextSize(label).x
                      + ImGui::GetStyle().FramePadding.x * 2.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - width - 24.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, Mix(kWarning, {0, 0, 0, 1}, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Mix(kWarning, {0, 0, 0, 1}, 0.40f));
            if (ImGui::Button(label))
            {
                OnClose(context);
                context.ShowSettings = false;
                context.Commands.Execute("Editor.Restart");
            }
            ImGui::PopStyleColor(2);
        }
    }

    void SettingsOverlay::DrawPage(EditorContext& context)
    {
        switch (m_Page)
        {
            case Page::eProjectGeneral: DrawProjectGeneralPage(context); break;
            case Page::eProjectRenderer: DrawProjectRendererPage(context); break;
            case Page::eEditorGeneral: DrawEditorGeneralPage(context); break;
            case Page::eAppearance: DrawAppearancePage(context); break;
            case Page::eViewport: DrawViewportPage(context); break;
            case Page::eGraphics: DrawGraphicsPage(context); break;
            case Page::eShortcuts: DrawShortcutsPage(context); break;
            case Page::eCameraControls: DrawCameraControlsPage(context); break;
        }
    }

    //------------------------------------------------------------------------
    // Project -----------------------------------------------------------------
    //------------------------------------------------------------------------

    void SettingsOverlay::DrawProjectGeneralPage(EditorContext& context)
    {
        auto project = EditorProject::Active();
        if (!project)
        {
            m_Page = Page::eEditorGeneral;
            return;
        }
        auto& config = project->RuntimeProject()->Config();
        PageTitle("Project", "Identity and layout of the open project. Saved to its "
                             ".vproj file with Save Project.");

        BeginCard("##ProjectInfo", "Metadata");
        if (BeginRows("##ProjectInfoRows"))
        {
            Row("Name");
            if (UI::InputText("##Name", config.Metadata.Name)) project->MarkDirty();
            Row("Author");
            if (UI::InputText("##Author", config.Metadata.Author)) project->MarkDirty();
            Row("Description");
            if (UI::InputText("##Description", config.Metadata.Description))
                project->MarkDirty();
            ValueRow("Version", String(config.Metadata.Version).Raw());
            ValueRow("Location", String(project->Path()).Raw());
            EndRows();
        }
        EndCard();

        BeginCard("##ProjectPaths", "Content");
        if (BeginRows("##ProjectPathRows"))
        {
            Row("Asset Directory", "Mounted as Assets/, relative to the project.");
            if (UI::InputText("##Assets", config.AssetDirectory)) project->MarkDirty();
            Row("Start Scene", "Opened with the project, relative to it.");
            f32 buttonWidth = ImGui::CalcTextSize("Use Current").x
                            + ImGui::GetStyle().FramePadding.x * 2.0f;
            ImGui::SetNextItemWidth(-buttonWidth - ImGui::GetStyle().ItemSpacing.x);
            if (UI::InputText("##StartScene", config.StartScenePath)) project->MarkDirty();
            ImGui::SameLine();
            ImGui::BeginDisabled(context.Scenes.ScenePath().Empty());
            if (ImGui::Button("Use Current"))
            {
                config.StartScenePath = RelativeTo(context.Scenes.ScenePath(), project->Path());
                project->MarkDirty();
            }
            ImGui::EndDisabled();
            Row("Script Module");
            if (UI::InputText("##ScriptModule", config.ScriptModulePath))
                project->MarkDirty();
            EndRows();
        }
        EndCard();

        ImGui::BeginDisabled(!project->IsDirty());
        if (ImGui::Button("Save Project", {140.0f, 0.0f}))
            SetStatus(project->Flush() ? String("Project saved")
                                       : String("Can't save the project (see the console)"));
        ImGui::EndDisabled();
        if (project->IsDirty())
        {
            ImGui::SameLine();
            ImGui::TextColored(kWarning, "Unsaved changes");
        }
    }

    void SettingsOverlay::DrawProjectRendererPage(EditorContext&)
    {
        auto project = EditorProject::Active();
        if (!project)
        {
            m_Page = Page::eEditorGeneral;
            return;
        }
        auto& renderer = project->RuntimeProject()->Config().RendererSettings;
        PageTitle("Project Renderer",
                  "How the game renders when it runs on its own (the runtime "
                  "player). The editor's own display is under Graphics.");

        BeginCard("##ProjectRenderer", "Output");
        if (BeginRows("##ProjectRendererRows"))
        {
            Row("V-Sync");
            if (ImGui::Checkbox("##VSync", &renderer.VSync)) project->MarkDirty();

            Row("Anti-aliasing");
            const char* msaa[]  = {"Off", "2x MSAA", "4x MSAA", "8x MSAA"};
            usize       samples = renderer.AntiAliasingSamples;
            int         index   = samples >= 8 ? 3 : samples >= 4 ? 2 : samples >= 2 ? 1 : 0;
            if (ImGui::Combo("##MSAA", &index, msaa, IM_ARRAYSIZE(msaa)))
            {
                constexpr usize kSamples[] = {0, 2, 4, 8};
                renderer.AntiAliasingSamples = kSamples[index];
                project->MarkDirty();
            }

            Row("Target Frame Rate", "0 = unlimited.");
            int fps = static_cast<int>(renderer.TargetFrameRate);
            if (ImGui::SliderInt("##FPS", &fps, 0, 360, fps == 0 ? "Unlimited" : "%d fps"))
            {
                renderer.TargetFrameRate = static_cast<usize>(fps);
                project->MarkDirty();
            }

            Row("Resolution");
            int resolution[2] = {static_cast<int>(renderer.Resolution.x),
                                 static_cast<int>(renderer.Resolution.y)};
            if (ImGui::InputInt2("##Resolution", resolution))
            {
                renderer.Resolution.x = static_cast<u32>(resolution[0] > 1 ? resolution[0] : 1);
                renderer.Resolution.y = static_cast<u32>(resolution[1] > 1 ? resolution[1] : 1);
                project->MarkDirty();
            }
            EndRows();
        }
        EndCard();

        ImGui::BeginDisabled(!project->IsDirty());
        if (ImGui::Button("Save Project", {140.0f, 0.0f}))
            SetStatus(project->Flush() ? String("Project saved")
                                       : String("Can't save the project (see the console)"));
        ImGui::EndDisabled();
    }

    //------------------------------------------------------------------------
    // Editor ------------------------------------------------------------------
    //------------------------------------------------------------------------

    void SettingsOverlay::DrawEditorGeneralPage(EditorContext& context)
    {
        auto& preferences = context.Preferences;
        PageTitle("General", "Startup and saving behaviour of the editor.");

        BeginCard("##Startup", "Startup");
        if (BeginRows("##StartupRows"))
        {
            Row("Reopen Last Project",
                "Open the most recent project when Nexus starts (unless one is "
                "given with --project).");
            ImGui::Checkbox("##OpenLast", &preferences.OpenLastProject);
            EndRows();
        }
        EndCard();

        BeginCard("##Saving", "Saving");
        if (BeginRows("##SavingRows"))
        {
            Row("Auto-save Scene",
                "Saves the open scene to its file periodically, while editing "
                "(never during Play or Simulate).");
            ImGui::Checkbox("##AutoSave", &preferences.AutoSave);
            Row("Interval");
            ImGui::BeginDisabled(!preferences.AutoSave);
            ImGui::SliderFloat("##AutoSaveInterval", &preferences.AutoSaveMinutes, 1.0f,
                               60.0f, "%.0f min");
            ImGui::EndDisabled();

            Row("Auto-compile Scripts",
                "Rebuild the C# scripts when a source file changes. A build that "
                "finishes during Play is loaded when the scene stops; a failed "
                "build keeps the scripts already loaded.");
            ImGui::Checkbox("##AutoCompileScripts", &preferences.AutoCompileScripts);
            EndRows();
        }
        EndCard();

        BeginCard("##Recent", "Recent Projects");
        if (preferences.RecentProjects.Empty()) ImGui::TextDisabled("No recent projects.");
        usize forget = static_cast<usize>(-1);
        for (usize i = 0; i < preferences.RecentProjects.Size(); ++i)
        {
            const auto& recent = preferences.RecentProjects[i];
            ImGui::PushID(static_cast<int>(i));
            ImGui::BeginGroup();
            ImGui::TextUnformatted(recent.Name.Raw());
            if (recent.Name == preferences.StartupProject) Badge("STARTUP", kAccent);
            ImGui::TextDisabled("%s", recent.Path.Raw());
            ImGui::EndGroup();
            ImGui::SameLine(ImGui::GetContentRegionMax().x - 70.0f);
            if (ImGui::SmallButton("Forget")) forget = i;
            ImGui::PopID();
            if (i + 1 < preferences.RecentProjects.Size()) ImGui::Separator();
        }
        if (forget < preferences.RecentProjects.Size())
            preferences.RecentProjects.Erase(preferences.RecentProjects.begin() + forget);
        EndCard();
    }

    void SettingsOverlay::DrawAppearancePage(EditorContext& context)
    {
        auto& preferences = context.Preferences;
        PageTitle("Appearance", "Look of the editor's interface.");

        BeginCard("##Theme", "Theme");
        if (BeginRows("##ThemeRows"))
        {
            Row("Color Theme");
            if (ImGui::BeginCombo("##Theme", ThemeName(preferences.Theme)))
            {
                for (int i = 0; i <= static_cast<int>(EditorTheme::eClassic); ++i)
                {
                    auto theme = static_cast<EditorTheme>(i);
                    if (ImGui::Selectable(ThemeName(theme), preferences.Theme == theme))
                    {
                        preferences.Theme = theme;
                        context.ApplyPreferences();
                    }
                }
                ImGui::EndCombo();
            }

            Row("Interface Scale", "Scales text and spacing. Applied when you let go "
                                   "of the slider.");
            ImGui::SliderFloat("##Scale", &preferences.UIScale, 0.75f, 2.0f, "%.2fx");
            if (ImGui::IsItemDeactivatedAfterEdit()) context.ApplyPreferences();
            EndRows();
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Preview");
        ImGui::Button("Button");
        ImGui::SameLine();
        static bool  check  = true;
        static float slider = 0.6f;
        ImGui::Checkbox("Checkbox", &check);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(160.0f);
        ImGui::SliderFloat("##PreviewSlider", &slider, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::TextColored(kAccent, "Accent");
        EndCard();

        if (ImGui::Button("Reset Appearance"))
        {
            preferences.Theme   = EditorTheme::eVortexDark;
            preferences.UIScale = 1.0f;
            context.ApplyPreferences();
        }
    }

    void SettingsOverlay::DrawViewportPage(EditorContext& context)
    {
        auto& viewport = context.Preferences.Viewport;
        PageTitle("Viewport", "The scene view's camera projection and overlays.");

        BeginCard("##Projection", "Projection");
        if (BeginRows("##ProjectionRows"))
        {
            Row("Field of View");
            ImGui::SliderFloat("##FOV", &viewport.FieldOfView, 30.0f, 120.0f, "%.0f deg");
            Row("Near Plane");
            ImGui::DragFloat("##Near", &viewport.Near, 0.01f, 0.001f, viewport.Far * 0.5f,
                             "%.3f");
            Row("Far Plane");
            ImGui::DragFloat("##Far", &viewport.Far, 1.0f, viewport.Near * 2.0f, 100000.0f,
                             "%.0f");
            EndRows();
        }
        EndCard();

        BeginCard("##Overlays", "Overlays");
        if (BeginRows("##OverlayRows"))
        {
            Row("Ground Grid", "Also View > Show Grid.");
            ImGui::Checkbox("##Grid", &viewport.ShowGrid);
            EndRows();
        }
        EndCard();

        if (ImGui::Button("Reset Viewport")) viewport = ViewportSettings{};
    }

    //------------------------------------------------------------------------
    // Graphics ----------------------------------------------------------------
    //------------------------------------------------------------------------

    void SettingsOverlay::DrawGraphicsPage(EditorContext& context)
    {
        auto&       preferences = context.Preferences;
        auto&       device      = Renderer::Device();
        const auto& adapters    = device.Adapters();
        u32         activeIndex = device.ActiveAdapter();
        const RHI::AdapterInfo* active
            = activeIndex < adapters.Size() ? &adapters[activeIndex] : nullptr;
        char buffer[128];

        PageTitle("GPU & Display", "Which graphics API and GPU the editor renders "
                                   "with, and how it presents frames.");

        // Adapter choice ---------------------------------------------------
        BeginCard("##Adapter", "Graphics Adapter");
        if (BeginRows("##AdapterRows"))
        {
            Row("Renderer API", "The graphics API the renderer runs on. The device "
                                "is created at startup: a change applies after a "
                                "restart.");
            RHI::API selectedAPI = Renderer::ActiveAPI();
            RHI::ParseAPI(preferences.RendererAPI, selectedAPI);
            if (ImGui::BeginCombo("##RendererAPI", RHI::APIName(selectedAPI)))
            {
                for (RHI::API api : RHI::kAPIs)
                {
                    ImGui::PushID(static_cast<int>(api));
                    if (ImGui::Selectable(RHI::APIName(api), api == selectedAPI))
                        preferences.RendererAPI = String(RHI::APIName(api));
                    if (api == selectedAPI) ImGui::SetItemDefaultFocus();
                    if (api == Renderer::ActiveAPI())
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(kGood, "in use");
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            Row("GPU", "The device is picked at startup: a change applies after "
                       "a restart.");
            String preview = preferences.PreferredGpu.Empty()
                               ? String("Automatic (best available)")
                               : preferences.PreferredGpu;
            if (ImGui::BeginCombo("##GPU", preview.Raw()))
            {
                if (ImGui::Selectable("Automatic (best available)",
                                      preferences.PreferredGpu.Empty()))
                    preferences.PreferredGpu = String{};
                ImGui::Separator();
                for (usize i = 0; i < adapters.Size(); ++i)
                {
                    const auto& adapter = adapters[i];
                    ImGui::PushID(static_cast<int>(i));
                    ImGui::BeginDisabled(!adapter.Suitable);
                    std::snprintf(buffer, sizeof(buffer), "%s  (%s)", adapter.Name.Raw(),
                                  RHI::AdapterTypeName(adapter.Type));
                    if (ImGui::Selectable(buffer, preferences.PreferredGpu == adapter.Name))
                        preferences.PreferredGpu = adapter.Name;
                    ImGui::EndDisabled();
                    if (!adapter.Suitable
                        && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip("Unsupported: %s", adapter.UnsuitableReason.Raw());
                    if (i == activeIndex)
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(kGood, "in use");
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            Row("V-Sync", "Wait for the display's refresh: no tearing, capped frame "
                          "rate. Off uses mailbox/immediate presentation.");
            if (ImGui::Checkbox("##EditorVSync", &preferences.VSync))
                context.ApplyPreferences();

            std::snprintf(buffer, sizeof(buffer), "%u", device.FramesInFlight());
            ValueRow("Frames in Flight", buffer);
            EndRows();
        }
        if (m_StartupGpu != preferences.PreferredGpu)
        {
            ImGui::Spacing();
            ImGui::TextColored(kWarning, "Restart Nexus to switch to %s.",
                               preferences.PreferredGpu.Empty()
                                   ? "the automatically chosen GPU"
                                   : preferences.PreferredGpu.Raw());
        }
        if (m_StartupAPI != preferences.RendererAPI)
        {
            ImGui::Spacing();
            ImGui::TextColored(kWarning, "Restart Nexus to render with %s.",
                               preferences.RendererAPI.Raw());
        }
        EndCard();

        // The running GPU -------------------------------------------------
        if (active)
        {
            BeginCard("##ActiveGpu", "Active GPU");
            PushFontScale(1.25f);
            ImGui::TextUnformatted(active->Name.Raw());
            ImGui::PopFont();
            Badge(RHI::AdapterTypeName(active->Type), kAccent);
            ImGui::Spacing();
            if (BeginRows("##GpuInfoRows"))
            {
                ValueRow("Vendor", active->Vendor.Raw());
                std::snprintf(buffer, sizeof(buffer), "0x%04X : 0x%04X", active->VendorID,
                              active->DeviceID);
                ValueRow("Vendor : Device ID", buffer);
                ValueRow("Driver", active->Driver.Raw());
                std::snprintf(buffer, sizeof(buffer), "%s %s", RHI::APIName(Renderer::ActiveAPI()), active->APIVersion.Raw());
                ValueRow("Graphics API", buffer);
                FormatBytes(buffer, sizeof(buffer), active->DedicatedMemory);
                ValueRow("Dedicated Memory", buffer);
                FormatBytes(buffer, sizeof(buffer), active->SharedMemory);
                ValueRow("Shared Memory", buffer);

                Row("VRAM Usage", "This process's GPU memory against the budget the "
                                  "driver gives it.");
                RHI::MemoryStats memory = Renderer::MemoryStats();
                if (memory.DeviceLocalBudget > 0)
                {
                    f32 fraction = static_cast<f32>(memory.DeviceLocalUsage)
                                 / static_cast<f32>(memory.DeviceLocalBudget);
                    char used[32], budget[32];
                    FormatBytes(used, sizeof(used), memory.DeviceLocalUsage);
                    FormatBytes(budget, sizeof(budget), memory.DeviceLocalBudget);
                    std::snprintf(buffer, sizeof(buffer), "%s / %s", used, budget);
                    ImVec4 color = fraction > 0.9f ? kBad : fraction > 0.7f ? kWarning : kAccent;
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
                    ImGui::ProgressBar(fraction, {-FLT_MIN, 0.0f}, buffer);
                    ImGui::PopStyleColor();
                }
                else ImGui::TextDisabled("Not reported by the driver");
                EndRows();
            }
            EndCard();
        }

        // Performance -----------------------------------------------------
        BeginCard("##Performance", "Performance");
        {
            f32 minimum = 1e9f, maximum = 0.0f, sum = 0.0f;
            usize samples = 0;
            for (f32 time : m_FrameTimes)
            {
                if (time <= 0.0f) continue;
                minimum = time < minimum ? time : minimum;
                maximum = time > maximum ? time : maximum;
                sum += time;
                ++samples;
            }
            f32 average = samples ? sum / static_cast<f32>(samples) : 0.0f;

            PushFontScale(1.6f);
            ImGui::TextColored(kAccent, "%.0f", ImGui::GetIO().Framerate);
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("fps   %.2f ms avg   %.2f min   %.2f max", average,
                                samples ? minimum : 0.0f, maximum);

            ImGui::PushStyleColor(ImGuiCol_PlotLines, kAccent);
            ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                  Mix(ImGui::GetStyleColorVec4(ImGuiCol_FrameBg), {0, 0, 0, 1}, 0.3f));
            ImGui::PlotLines("##FrameTimes", m_FrameTimes.Raw(), static_cast<int>(kFrameHistory),
                             static_cast<int>(m_FrameOffset), "frame time (ms)", 0.0f,
                             maximum > 0.0f ? maximum * 1.25f : 33.3f, {-FLT_MIN, 70.0f});
            ImGui::PopStyleColor(2);
        }
        EndCard();

        // Capabilities ----------------------------------------------------
        BeginCard("##Capabilities", "Capabilities");
        if (BeginRows("##CapabilityRows"))
        {
            const auto& limits   = device.Limits();
            const auto& features = device.Features();
            std::snprintf(buffer, sizeof(buffer), "%u px", limits.MaxTextureDimension2D);
            ValueRow("Max Texture Size", buffer);
            std::snprintf(buffer, sizeof(buffer), "%u", limits.MaxColorAttachments);
            ValueRow("Max Color Attachments", buffer);
            std::snprintf(buffer, sizeof(buffer), "%u", limits.MaxBoundBindGroups);
            ValueRow("Max Bound Bind Groups", buffer);
            std::snprintf(buffer, sizeof(buffer), "%u bytes", limits.MaxPushConstantSize);
            ValueRow("Push Constants", buffer);
            std::snprintf(buffer, sizeof(buffer), "%.0fx", limits.MaxSamplerAnisotropy);
            ValueRow("Max Anisotropy", buffer);

            auto feature = [](const char* name, bool supported)
            { ValueRow(name, supported ? "Supported" : "Not supported", supported ? kGood : kBad); };
            feature("Anisotropic Filtering", features.SamplerAnisotropy);
            feature("BC Texture Compression", features.TextureCompressionBC);
            feature("Wireframe Fill Mode", features.FillModeNonSolid);
            feature("Timestamp Queries", features.TimestampQueries);
            EndRows();
        }
        EndCard();

        // Every adapter ---------------------------------------------------
        BeginCard("##AllAdapters", "Detected Adapters");
        if (ImGui::BeginTable("##AdapterTable", 4,
                              ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp
                                  | ImGuiTableFlags_BordersInnerH))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.40f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 0.18f);
            ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthStretch, 0.14f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 0.28f);
            ImGui::TableHeadersRow();
            for (usize i = 0; i < adapters.Size(); ++i)
            {
                const auto& adapter = adapters[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(adapter.Name.Raw());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(RHI::AdapterTypeName(adapter.Type));
                ImGui::TableNextColumn();
                FormatBytes(buffer, sizeof(buffer), adapter.DedicatedMemory);
                ImGui::TextUnformatted(buffer);
                ImGui::TableNextColumn();
                if (i == activeIndex) ImGui::TextColored(kGood, "Active");
                else if (adapter.Suitable) ImGui::TextUnformatted("Available");
                else ImGui::TextColored(kBad, "Unsupported: %s", adapter.UnsuitableReason.Raw());
            }
            ImGui::EndTable();
        }
        EndCard();
    }

    //------------------------------------------------------------------------
    // Input -------------------------------------------------------------------
    //------------------------------------------------------------------------

    bool SettingsOverlay::DrawBindingButton(const char* id, i32 chord, bool capturing,
                                            f32 width)
    {
        char label[96];
        if (capturing)
            std::snprintf(label, sizeof(label), "Press keys...%s", id);
        else if (chord)
            std::snprintf(label, sizeof(label), "%s%s", CommandRegistry::ShortcutText(chord).Raw(), id);
        else std::snprintf(label, sizeof(label), "Unbound%s", id);

        int colors = 0;
        if (capturing)
        {
            f32    pulse = 0.5f + 0.5f * std::sin(static_cast<f32>(ImGui::GetTime()) * 6.0f);
            ImVec4 color = kAccent;
            color.w      = 0.30f + 0.35f * pulse;
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
            colors = 2;
        }
        else if (!chord)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
            colors = 1;
        }
        bool clicked = ImGui::Button(label, {width, 0.0f});
        ImGui::PopStyleColor(colors);
        if (capturing) ImGui::SetItemTooltip("Press the new keys; Esc cancels.");
        return clicked && !capturing && m_CaptureEndedOn != ImGui::GetFrameCount();
    }

    void SettingsOverlay::UpdateCapture(EditorContext& context)
    {
        if (m_Capture == Capture::eNone) return;
        context.Commands.SetShortcutsEnabled(false);

        const ImGuiIO& io   = ImGui::GetIO();
        const int      mods = io.KeyMods
                       & (ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiMod_Super);
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_GamepadStart; ++k)
        {
            auto key = static_cast<ImGuiKey>(k);
            if (m_Capture == Capture::eCommand && IsModifierKey(key)) continue;
            if (!ImGui::IsKeyPressed(key, false)) continue;

            if (key == ImGuiKey_Escape && !mods)
            {
                EndCapture();
                SetStatus("Rebinding cancelled");
                return;
            }

            if (m_Capture == Capture::eCommand)
            {
                int chord = key | mods;
                if (EditorCommand* other = context.Commands.FindByShortcut(chord, m_CaptureCommand))
                {
                    SetStatus(Prism::Format("{} moved from '{}'",
                                            StringView(CommandRegistry::ShortcutText(chord).Raw()),
                                            other->Label));
                    other->Shortcut = 0;
                }
                else
                    SetStatus(Prism::Format("Bound {}", StringView(CommandRegistry::ShortcutText(chord).Raw())));
                context.Commands.SetShortcut(m_CaptureCommand, chord);
                context.StoreShortcuts();
            }
            else if (m_CaptureKey)
            {
                *m_CaptureKey = key;
                SetStatus(Prism::Format("Bound {}", StringView(ImGui::GetKeyName(key))));
            }
            EndCapture();
            return;
        }
    }

    void SettingsOverlay::EndCapture()
    {
        if (m_Capture == Capture::eNone) return;
        m_Capture        = Capture::eNone;
        m_CaptureKey     = nullptr;
        m_CaptureCommand = String{};
        m_CaptureEndedOn = ImGui::GetFrameCount();
    }

    void SettingsOverlay::DrawShortcutsPage(EditorContext& context)
    {
        auto& commands = context.Commands;
        PageTitle("Keyboard Shortcuts",
                  "Click a shortcut, then press the new key combination. A "
                  "combination already in use moves to the command you're "
                  "editing.");

        f32 resetWidth = ImGui::CalcTextSize("Reset All").x
                       + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - resetWidth
                                - ImGui::GetStyle().ItemSpacing.x);
        UI::InputTextWithHint("##ShortcutFilter", "Search commands or keys...",
                              m_ShortcutFilter);
        ImGui::SameLine();
        if (ImGui::Button("Reset All"))
        {
            EndCapture();
            commands.ResetAllShortcuts();
            context.StoreShortcuts();
            SetStatus("All shortcuts reset to their defaults");
        }
        ImGui::Spacing();

        // Categories in menu order; commands without a menu go to "General".
        Vector<String> categories;
        for (const auto& command : commands.Commands())
        {
            String category = command.Menu.Empty() ? String("General") : command.Menu;
            bool   known    = false;
            for (const auto& existing : categories) known = known || existing == category;
            if (!known) categories.PushBack(category);
        }

        bool anyShown = false;
        for (const auto& category : categories)
        {
            // Ids first: rebinding below doesn't reorder, but stay safe.
            Vector<String> ids;
            for (const auto& command : commands.Commands())
            {
                String commandCategory
                    = command.Menu.Empty() ? String("General") : command.Menu;
                if (commandCategory != category) continue;
                String keys = CommandRegistry::ShortcutText(command.Shortcut);
                if (!ContainsIgnoreCase(command.Label, m_ShortcutFilter)
                    && !ContainsIgnoreCase(command.Id, m_ShortcutFilter)
                    && !ContainsIgnoreCase(keys, m_ShortcutFilter))
                    continue;
                ids.PushBack(command.Id);
            }
            if (ids.Empty()) continue;
            anyShown = true;

            ImGui::PushID(category.Raw());
            BeginCard("##Category", category.Raw());
            if (ImGui::BeginTable("##Bindings", 3, ImGuiTableFlags_SizingStretchProp))
            {
                const auto& style   = ImGui::GetStyle();
                const f32   actions = ImGui::CalcTextSize("Reset").x
                                  + ImGui::CalcTextSize("Clear").x
                                  + style.FramePadding.x * 4.0f + style.ItemSpacing.x;
                ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_WidthStretch, 0.55f);
                ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthStretch, 0.45f);
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, actions);
                for (const auto& id : ids)
                {
                    EditorCommand* command = commands.Find(id);
                    if (!command) continue;
                    ImGui::PushID(id.Raw());
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(command->Label.Raw());
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", command->Id.Raw());
                    if (command->IsRebound()) Badge("CUSTOM", kAccent);

                    ImGui::TableNextColumn();
                    bool capturing = m_Capture == Capture::eCommand && m_CaptureCommand == id;
                    if (DrawBindingButton("##Binding", command->Shortcut, capturing, -FLT_MIN))
                    {
                        m_Capture        = Capture::eCommand;
                        m_CaptureCommand = id;
                        m_CaptureKey     = nullptr;
                    }
                    if (command->Shortcut && commands.FindByShortcut(command->Shortcut, id))
                    {
                        ImGui::TextColored(kBad, "Conflicts with '%s'",
                                           commands.FindByShortcut(command->Shortcut, id)
                                               ->Label.Raw());
                    }

                    ImGui::TableNextColumn();
                    ImGui::BeginDisabled(!command->IsRebound());
                    if (ImGui::SmallButton("Reset"))
                    {
                        commands.ResetShortcut(id);
                        context.StoreShortcuts();
                    }
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!command->Shortcut);
                    if (ImGui::SmallButton("Clear"))
                    {
                        commands.SetShortcut(id, 0);
                        context.StoreShortcuts();
                    }
                    ImGui::EndDisabled();
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            EndCard();
            ImGui::PopID();
        }
        if (!anyShown) ImGui::TextDisabled("No command matches '%s'.", m_ShortcutFilter.Raw());
    }

    void SettingsOverlay::DrawCameraControlsPage(EditorContext& context)
    {
        auto& camera = context.Preferences.Camera;
        PageTitle("Camera Controls",
                  "Flying the scene camera: hold the right mouse button in the "
                  "viewport to look around, and move with these keys.");

        BeginCard("##CameraKeys", "Movement Keys");
        if (BeginRows("##CameraKeyRows"))
        {
            struct Binding
            {
                const char* Label;
                i32*        Key;
            };
            Binding bindings[] = {
                {"Move Forward", &camera.Forward},  {"Move Backward", &camera.Backward},
                {"Move Left", &camera.Left},        {"Move Right", &camera.Right},
                {"Move Up", &camera.Up},            {"Move Down", &camera.Down},
                {"Move Faster (hold)", &camera.Fast}, {"Turn Left", &camera.TurnLeft},
                {"Turn Right", &camera.TurnRight},
            };
            for (auto& binding : bindings)
            {
                ImGui::PushID(binding.Label);
                Row(binding.Label);
                bool capturing = m_Capture == Capture::eCameraKey && m_CaptureKey == binding.Key;
                if (DrawBindingButton("##Key", *binding.Key, capturing, -FLT_MIN))
                {
                    m_Capture        = Capture::eCameraKey;
                    m_CaptureKey     = binding.Key;
                    m_CaptureCommand = String{};
                }
                ImGui::PopID();
            }
            EndRows();
        }
        EndCard();

        BeginCard("##CameraFeel", "Feel");
        if (BeginRows("##CameraFeelRows"))
        {
            Row("Move Speed", "Units per second.");
            ImGui::SliderFloat("##Speed", &camera.MoveSpeed, 0.1f, 50.0f, "%.1f u/s",
                               ImGuiSliderFlags_Logarithmic);
            Row("Fast Multiplier");
            ImGui::SliderFloat("##Fast", &camera.FastMultiplier, 1.0f, 50.0f, "%.1fx");
            Row("Look Sensitivity");
            ImGui::SliderFloat("##Look", &camera.LookSensitivity, 1.0f, 50.0f, "%.1f");
            Row("Invert Vertical Look");
            ImGui::Checkbox("##InvertY", &camera.InvertY);
            EndRows();
        }
        EndCard();

        if (ImGui::Button("Reset Camera Controls"))
        {
            EndCapture();
            camera = EditorCameraControls{};
            SetStatus("Camera controls reset");
        }
    }
} // namespace Vortex::Editor
