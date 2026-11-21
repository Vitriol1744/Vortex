/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
// Imported directly, not only through Vortex.Renderer's re-export: CMake
// otherwise maps this module to the renderer's own BMI instead of the one
// built with Horizon's flags, and clang crashes reading the mix.
import Vortex.Renderer.RenderGraph;

#include <Horizon/LauncherLayer.hpp>

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Renderer/Image.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>
#include <Vortex/Window/Window.hpp>

#include <imgui.h>

#include <cmath>
#include <cstdio>
#include <ctime>

import Prism.Containers.Array;
import Prism.String.Formatter;
import Horizon.Http;
import Horizon.Projects;
import Horizon.Widgets;

namespace Horizon
{
    using namespace Ui;

    namespace
    {
        constexpr float kSidebarWidth = 236.0f;
        constexpr float kPagePadX     = 36.0f;
        constexpr float kPagePadY     = 30.0f;

        void CopyTo(char* buffer, usize size, StringView text)
        {
            std::snprintf(buffer, size, "%.*s", static_cast<int>(text.Size()), text.Raw());
        }

        String TimeAgo(i64 timestamp)
        {
            if (timestamp <= 0) return String("never");
            i64 seconds = static_cast<i64>(std::time(nullptr)) - timestamp;
            if (seconds < 60) return String("just now");
            if (seconds < 3600) return Format("{} min ago", seconds / 60);
            if (seconds < 86400) return Format("{} h ago", seconds / 3600);
            if (seconds < 86400 * 2) return String("yesterday");
            if (seconds < 86400 * 30) return Format("{} days ago", seconds / 86400);
            char        text[32];
            std::time_t t = static_cast<std::time_t>(timestamp);
            std::strftime(text, sizeof(text), "%b %d, %Y", std::localtime(&t));
            return String(text);
        }

        bool ContainsCaseless(StringView haystack, StringView needle)
        {
            if (needle.Empty()) return true;
            auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c - 'A' + 'a') : c; };
            for (usize i = 0; i + needle.Size() <= haystack.Size(); ++i)
            {
                usize j = 0;
                while (j < needle.Size() && lower(haystack[i + j]) == lower(needle[j])) ++j;
                if (j == needle.Size()) return true;
            }
            return false;
        }

        /// Up to two letters for a project thumbnail ("My Game" -> "MG").
        String Initials(StringView name)
        {
            String initials;
            bool   start = true;
            for (char c : name)
            {
                bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                           || (c >= '0' && c <= '9');
                if (letter && start && initials.Size() < 2)
                    initials += static_cast<char>(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c);
                start = !letter;
            }
            return initials.Empty() ? String("?") : initials;
        }

        /// Opens the system file manager on `path` without blocking the UI.
        void Reveal(const Path& path)
        {
            String target(IsDirectory(path) ? path.StrView() : path.ParentPath().StrView());
#ifdef _WIN32
            Vector<String> argv{String("explorer"), target};
#elif defined(__APPLE__)
            Vector<String> argv{String("open"), target};
#else
            Vector<String> argv{String("xdg-open"), target};
#endif
            std::thread([argv = Move(argv)] { RunAndWait(argv); }).detach();
        }

        /// Width of a Ui::Button with this label and icon.
        float ButtonWidth(const char* label, const char* icon = nullptr)
        {
            FontScope font(GetFonts().Medium, Size::Body);
            String    text = icon ? Format("{}  {}", icon, label) : String(label);
            return ImGui::CalcTextSize(text.Raw()).x + 32.0f;
        }

        /// Places the next item `width` pixels from the right edge.
        void AlignRight(float width)
        {
            float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - width;
            ImGui::SetCursorPosX(x);
        }

        /// A text input styled to fill the width, with a hint.
        bool Input(const char* id, char* buffer, usize size, const char* hint = nullptr,
                   float width = -1.0f)
        {
            ImGui::SetNextItemWidth(width);
            return hint ? ImGui::InputTextWithHint(id, hint, buffer, size)
                        : ImGui::InputText(id, buffer, size);
        }

        /// An input with a folder-picker button.
        bool FolderInput(const char* id, char* buffer, usize size)
        {
            constexpr float kButton = 38.0f;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - kButton
                                    - ImGui::GetStyle().ItemSpacing.x);
            bool changed = ImGui::InputText(id, buffer, size);
            ImGui::SameLine();
            ImGui::PushID(id);
            float y = ImGui::GetCursorPosY();
            ImGui::SetCursorPosY(y + (ImGui::GetFrameHeight() - kButton) * 0.5f);
            if (IconButton("browse", Icon::FolderOpen, "Browse...", Color::Muted, kButton))
            {
                if (auto folder = Vortex::Window::OpenFolder(Path(buffer)))
                {
                    CopyTo(buffer, size, folder->StrView());
                    changed = true;
                }
            }
            ImGui::PopID();
            return changed;
        }

        /// A rounded gradient square with an icon (engine rows, banners).
        void IconBadge(const char* icon, float size, ImVec4 from, ImVec4 to)
        {
            ImVec2      pos  = ImGui::GetCursorScreenPos();
            ImDrawList* draw = ImGui::GetWindowDrawList();
            GradientRect(draw, pos, {pos.x + size, pos.y + size}, from, to, size * 0.28f);
            FontScope font(GetFonts().Regular, size * 0.45f);
            ImVec2    text = ImGui::CalcTextSize(icon);
            draw->AddText({pos.x + (size - text.x) * 0.5f, pos.y + (size - text.y) * 0.5f},
                          IM_COL32_WHITE, icon);
            ImGui::Dummy({size, size});
        }
    } // namespace

    LauncherLayer::LauncherLayer()
        : Layer("Horizon")
    {
    }
    LauncherLayer::~LauncherLayer()
    {
        // An install in progress is cancelled (its destructor joins).
        m_Job.Reset();
        if (m_FetchThread.joinable()) m_FetchThread.join();
    }

    void LauncherLayer::OnAttach()
    {
        // imgui.ini in the working directory belongs to Nexus; the launcher's
        // layout is fixed, so it neither reads nor writes one.
        ImGui::GetIO().IniFilename = nullptr;
        LoadFonts();
        ApplyTheme();

        auto& app = *Vortex::Application::Get();
        if (Exists("assets/icons/icon.bmp"))
        {
            Vortex::Image icon("assets/icons/icon.bmp");
            app.Window().SetIcon(icon);
        }
        app.Window().ShowCursor();

        // `--page engines` (or settings) opens on that page; `--page new`
        // opens the New project dialog (a desktop shortcut's action).
        StringView page = app.Arguments().Value("--page");
        if (page == "engines") m_Page = Page::eEngines;
        else if (page == "settings") m_Page = Page::eSettings;
        else if (page == "new") m_OpenNewProject = true;

        m_Settings = Settings::Load();
        LoadDraft();
        Rescan();
        CopyTo(m_NewName, sizeof(m_NewName), "New Project");
        CopyTo(m_NewLocation, sizeof(m_NewLocation), m_Settings.ProjectsDirectory.StrView());
        FetchManifest();
    }

    void LauncherLayer::OnUpdate(Vortex::Timestep)
    {
        if (!m_Job) return;

        // Smoothed download speed, sampled twice a second.
        auto now = Clock::now();
        if (now - m_SpeedTime >= std::chrono::milliseconds(500))
        {
            double seconds = std::chrono::duration<double>(now - m_SpeedTime).count();
            u64    bytes   = m_Job->BytesReceived();
            double sample  = bytes >= m_SpeedBytes ? double(bytes - m_SpeedBytes) / seconds : 0.0;
            m_BytesPerSecond = m_BytesPerSecond == 0.0 ? sample : m_BytesPerSecond * 0.6 + sample * 0.4;
            m_SpeedBytes     = bytes;
            m_SpeedTime      = now;
        }

        if (!m_Job->Finished()) return;
        if (m_Job->CurrentState() == InstallJob::State::eDone)
        {
            Notify(Format("Installed Vortex {}", m_Job->Target().Version));
            if (m_Settings.DefaultEngine.Empty())
            {
                m_Settings.DefaultEngine = m_Job->Target().Version;
                m_Draft.DefaultEngine    = m_Settings.DefaultEngine;
                SaveSettings();
            }
        }
        else Notify(m_Job->Error(), true);
        m_Job.Reset();
        Rescan();
    }

    void LauncherLayer::OnImGuiRender()
    {
        // Held for the frame: the fetch thread only takes it to publish.
        std::lock_guard lock(m_FetchMutex);
        HandleShortcuts();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
        constexpr auto kFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                              | ImGuiWindowFlags_NoSavedSettings
                              | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;
        ImGui::Begin("##Horizon", nullptr, kFlags);
        ImGui::PopStyleVar(3);

        // Sidebar.
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Color::Sidebar);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {18.0f, 22.0f});
        ImGui::BeginChild("##Sidebar", {kSidebarWidth, 0},
                          ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar);
        DrawSidebar();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        // The sidebar's edge.
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2      edge = ImGui::GetItemRectMax();
        draw->AddLine({edge.x, viewport->WorkPos.y}, {edge.x, edge.y}, U32(Color::Border));

        // Page.
        ImGui::SameLine(0.0f, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {kPagePadX, kPagePadY});
        ImGui::BeginChild("##Page", {0, 0}, ImGuiChildFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleVar();
        switch (m_Page)
        {
            case Page::eProjects: DrawProjectsPage(); break;
            case Page::eEngines: DrawEnginesPage(); break;
            case Page::eSettings: DrawSettingsPage(); break;
        }
        DrawNewProjectModal();
        DrawRemoveEngineModal();
        ImGui::EndChild();

        ImGui::End();
        DrawToasts();
    }

    void LauncherLayer::HandleShortcuts()
    {
        if (ImGui::GetIO().WantTextInput) return;
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N))
        {
            m_Page           = Page::eProjects;
            m_OpenNewProject = true;
        }
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O)) BrowseForProject();
        if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) FetchManifest();
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_1)) m_Page = Page::eProjects;
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_2)) m_Page = Page::eEngines;
        if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_3)) m_Page = Page::eSettings;
    }

    // -- Sidebar --------------------------------------------------------------

    void LauncherLayer::DrawSidebar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.0f, 6.0f});

        // Brand.
        float  time   = static_cast<float>(ImGui::GetTime());
        ImVec2 origin = ImGui::GetCursorScreenPos();
        Logo(ImGui::GetWindowDrawList(), {origin.x + 22.0f, origin.y + 22.0f}, 20.0f, time);
        ImGui::SetCursorScreenPos({origin.x + 54.0f, origin.y + 2.0f});
        ImGui::BeginGroup();
        {
            FontScope font(GetFonts().Bold, 21.0f);
            ImGui::TextUnformatted("Horizon");
        }
        {
            FontScope font(GetFonts().Regular, Size::Small);
            Muted("Vortex Launcher");
        }
        ImGui::EndGroup();
        ImGui::SetCursorScreenPos({origin.x, origin.y + 64.0f});

        {
            FontScope font(GetFonts().Medium, 11.5f);
            TextColored(Color::Faint, "MENU");
        }
        ImGui::Dummy({0, 2});
        DrawNavItem(Icon::Folder, "Projects", Page::eProjects, false);
        DrawNavItem(Icon::Cubes, "Engines", Page::eEngines, AvailableUpdate() != nullptr);
        DrawNavItem(Icon::Cog, "Settings", Page::eSettings, false);

        DrawSidebarFooter();
        ImGui::PopStyleVar();
    }

    void LauncherLayer::DrawNavItem(const char* icon, const char* label, Page page, bool badge)
    {
        bool   selected = m_Page == page;
        float  width    = ImGui::GetContentRegionAvail().x;
        ImVec2 pos      = ImGui::GetCursorScreenPos();
        ImGui::PushID(label);
        if (ImGui::InvisibleButton("##nav", {width, 40.0f})) m_Page = page;
        bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2      max  = {pos.x + width, pos.y + 40.0f};
        if (selected)
        {
            GradientRect(draw, pos, max, WithAlpha(Color::Accent, 0.22f),
                         WithAlpha(Color::AccentAlt, 0.04f), 10.0f);
            draw->AddRectFilled({pos.x, pos.y + 10.0f}, {pos.x + 3.0f, max.y - 10.0f},
                                U32(Color::AccentAlt), 2.0f);
        }
        else if (hovered) draw->AddRectFilled(pos, max, U32(Color::Surface), 10.0f);

        ImVec4    fg = selected ? Color::Text : hovered ? Color::Text : Color::Muted;
        FontScope font(GetFonts().Medium, Size::Body);
        float     textY = pos.y + (40.0f - ImGui::GetTextLineHeight()) * 0.5f;
        draw->AddText({pos.x + 16.0f, textY}, U32(selected ? Color::AccentAlt : fg), icon);
        draw->AddText({pos.x + 44.0f, textY}, U32(fg), label);
        if (badge)
        {
            float pulse = 0.65f + 0.35f * std::sin(static_cast<float>(ImGui::GetTime()) * 3.0f);
            draw->AddCircleFilled({max.x - 16.0f, pos.y + 20.0f}, 4.0f,
                                  U32(WithAlpha(Color::Success, pulse)));
        }
    }

    void LauncherLayer::DrawSidebarFooter()
    {
        // Pinned to the bottom: the current download, or the default engine.
        float  blockHeight = m_Job ? 132.0f : 86.0f;
        float  bottom      = ImGui::GetWindowHeight() - ImGui::GetStyle().WindowPadding.y;
        ImGui::SetCursorPosY(bottom - blockHeight);

        if (m_Job)
        {
            BeginCard("##sidedownload", Color::Surface, 14.0f);
            const char* what = "Downloading";
            switch (m_Job->CurrentState())
            {
                case InstallJob::State::eVerifying: what = "Verifying"; break;
                case InstallJob::State::eExtracting: what = "Unpacking"; break;
                default: break;
            }
            {
                FontScope font(GetFonts().Medium, Size::Small);
                TextColored(Color::AccentAlt, Format("{}  {}", Icon::Download, what));
                Text(Format("Vortex {}", m_Job->Target().Version));
            }
            Progress(m_Job->Progress(), {-1, 6.0f}, static_cast<float>(ImGui::GetTime()));
            FontScope font(GetFonts().Regular, 12.0f);
            Muted(Format("{} / s", FormatSize(static_cast<u64>(m_BytesPerSecond))));
            EndCard();
        }
        else
        {
            const EngineInstall* engine = PickInstall(m_Installs, {}, m_Settings);
            FontScope font(GetFonts().Regular, Size::Small);
            TextColored(Color::Faint, "DEFAULT ENGINE");
            ImGui::Dummy({0, 2});
            if (engine)
            {
                FontScope medium(GetFonts().Medium, Size::Body);
                TextColored(Color::Text, Format("{}  {}", Icon::Cube, engine->DisplayName));
            }
            else TextColored(Color::Warning, Format("{}  None installed", Icon::Warning));
            ImGui::Dummy({0, 6});
        }
        FontScope font(GetFonts().Regular, 12.0f);
        TextColored(Color::Faint, Format("{}  {}", Icon::Globe, HostPlatform()));
    }

    void LauncherLayer::DrawPageHeader(const char* title, StringView subtitle, float actionsWidth)
    {
        ImVec2 start = ImGui::GetCursorPos();
        float  room  = ImGui::GetContentRegionAvail().x - actionsWidth - 24.0f;
        ImGui::BeginGroup();
        {
            FontScope font(GetFonts().Bold, Size::Title);
            ImGui::TextUnformatted(title);
        }
        Muted(EllipsizeStart(subtitle, room));
        ImGui::EndGroup();
        float headerHeight = ImGui::GetItemRectSize().y;

        // Actions are vertically centred against the title block.
        ImGui::SetCursorPos(
            {start.x + ImGui::GetContentRegionAvail().x - actionsWidth,
             start.y + (headerHeight - 38.0f) * 0.5f});
    }

    // -- Projects -------------------------------------------------------------

    void LauncherLayer::DrawProjectsPage()
    {
        usize count = m_Settings.RecentProjects.Size();
        String subtitle = count == 0 ? String("Create a project, or open one from disk")
                        : count == 1 ? String("1 project")
                                     : Format("{} projects", count);

        float searchWidth = 240.0f;
        float openWidth   = ButtonWidth("Open", Icon::FolderOpen);
        float newWidth    = ButtonWidth("New project", Icon::Plus);
        float spacing     = 10.0f;
        DrawPageHeader("Projects", subtitle, searchWidth + openWidth + newWidth + spacing * 2);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {12.0f, 10.0f});
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
        String hint = Format("{}  Search projects", Icon::Search);
        Input("##search", m_Search, sizeof(m_Search), hint.Raw(), searchWidth);
        ImGui::PopStyleVar();
        ImGui::SameLine(0, spacing);
        if (Button("Open", Icon::FolderOpen)) BrowseForProject();
        Tooltip("Open a .vproj (Ctrl+O)");
        ImGui::SameLine(0, spacing);
        if (Button("New project", Icon::Plus, ButtonKind::ePrimary)) m_OpenNewProject = true;
        Tooltip("Ctrl+N");

        ImGui::Dummy({0, 22});

        // A responsive grid of tiles; the "new project" tile comes first.
        constexpr float kMinTile = 230.0f, kGap = 18.0f, kTileHeight = 206.0f;
        float           avail    = ImGui::GetContentRegionAvail().x;
        int             columns  = static_cast<int>((avail + kGap) / (kMinTile + kGap));
        if (columns < 1) columns = 1;
        float tileWidth = (avail - kGap * static_cast<float>(columns - 1)) / static_cast<float>(columns);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {kGap, kGap});
        int column = 0;
        auto next  = [&]
        {
            if (++column < columns) ImGui::SameLine();
            else column = 0;
        };

        DrawNewProjectTile(tileWidth, kTileHeight);
        next();
        usize shown = 0;
        for (usize i = 0; i < m_Settings.RecentProjects.Size(); ++i)
        {
            const auto& recent = m_Settings.RecentProjects[i];
            if (!ContainsCaseless(recent.Name, m_Search)
                && !ContainsCaseless(recent.Location.StrView(), m_Search))
                continue;
            DrawProjectTile(i, tileWidth, kTileHeight);
            next();
            ++shown;
        }
        if (column != 0) ImGui::NewLine();
        ImGui::PopStyleVar();

        if (m_Search[0] && shown == 0 && count > 0)
            Muted(Format("No projects match \"{}\".", StringView(m_Search)));
        else if (count == 0)
        {
            ImGui::Dummy({0, 8});
            Muted("Projects you create or open show up here. Double-click a project to open it,");
            Muted("right-click it for more.");
        }
    }

    void LauncherLayer::DrawNewProjectTile(float width, float height)
    {
        ImVec2 pos     = ImGui::GetCursorScreenPos();
        bool   clicked = ImGui::InvisibleButton("##newtile", {width, height});
        bool   hovered = ImGui::IsItemHovered();
        if (clicked) m_OpenNewProject = true;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2      max  = {pos.x + width, pos.y + height};
        if (hovered)
            GradientRect(draw, pos, max, WithAlpha(Color::Accent, 0.10f),
                         WithAlpha(Color::AccentAlt, 0.06f), 14.0f);
        // Dashed outline.
        ImU32 line  = U32(hovered ? WithAlpha(Color::Accent, 0.9f) : Color::Border);
        float dash  = 8.0f, gap = 6.0f;
        auto  dashed = [&](ImVec2 a, ImVec2 b)
        {
            float len = std::hypot(b.x - a.x, b.y - a.y);
            ImVec2 dir = {(b.x - a.x) / len, (b.y - a.y) / len};
            for (float t = 0; t < len; t += dash + gap)
            {
                float e = t + dash < len ? t + dash : len;
                draw->AddLine({a.x + dir.x * t, a.y + dir.y * t}, {a.x + dir.x * e, a.y + dir.y * e},
                              line, 1.5f);
            }
        };
        float r = 14.0f;
        dashed({pos.x + r, pos.y}, {max.x - r, pos.y});
        dashed({pos.x + r, max.y}, {max.x - r, max.y});
        dashed({pos.x, pos.y + r}, {pos.x, max.y - r});
        dashed({max.x, pos.y + r}, {max.x, max.y - r});
        draw->PathArcTo({pos.x + r, pos.y + r}, r, 3.14159f, 4.71239f, 8);
        draw->PathStroke(line, 0, 1.5f);
        draw->PathArcTo({max.x - r, pos.y + r}, r, 4.71239f, 6.28318f, 8);
        draw->PathStroke(line, 0, 1.5f);
        draw->PathArcTo({max.x - r, max.y - r}, r, 0.0f, 1.5708f, 8);
        draw->PathStroke(line, 0, 1.5f);
        draw->PathArcTo({pos.x + r, max.y - r}, r, 1.5708f, 3.14159f, 8);
        draw->PathStroke(line, 0, 1.5f);

        ImVec2 center = {pos.x + width * 0.5f, pos.y + height * 0.5f - 14.0f};
        draw->AddCircleFilled(center, 24.0f,
                              U32(hovered ? WithAlpha(Color::Accent, 0.9f) : Color::Surface), 32);
        {
            FontScope font(GetFonts().Bold, 20.0f);
            ImVec2    size = ImGui::CalcTextSize(Icon::Plus);
            draw->AddText({center.x - size.x * 0.5f, center.y - size.y * 0.5f},
                          U32(hovered ? ImVec4{1, 1, 1, 1} : Color::Muted), Icon::Plus);
        }
        FontScope font(GetFonts().Medium, Size::Body);
        const char* label = "New project";
        ImVec2      size  = ImGui::CalcTextSize(label);
        draw->AddText({center.x - size.x * 0.5f, center.y + 36.0f},
                      U32(hovered ? Color::Text : Color::Muted), label);
    }

    void LauncherLayer::DrawProjectTile(usize index, float width, float height)
    {
        const RecentProject recent = m_Settings.RecentProjects[index];
        bool                exists = Exists(recent.Location);

        ImGui::PushID(static_cast<int>(index));
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##tile", {width, height});
        bool hovered = ImGui::IsItemHovered();
        bool open    = exists && hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

        ImDrawList*     draw  = ImGui::GetWindowDrawList();
        ImVec2          max   = {pos.x + width, pos.y + height};
        constexpr float kThumb = 112.0f, kRound = 14.0f;
        float           time  = static_cast<float>(ImGui::GetTime());

        // Card.
        draw->AddRectFilled(pos, max, U32(hovered ? Color::SurfaceHi : Color::Surface), kRound);

        // Thumbnail: a colour from the name with a little vortex in it.
        ImVec4 base = ColorFor(recent.Name);
        if (!exists) base = Mix(base, Color::Faint, 0.8f);
        ImVec2 thumbMax = {max.x, pos.y + kThumb};
        GradientRect(draw, pos, thumbMax, Mix(base, Color::Background, 0.35f),
                     Mix(base, Color::Background, 0.75f), kRound, ImDrawFlags_RoundCornersTop);
        draw->PushClipRect(pos, thumbMax, true);
        ImVec2 swirl = {max.x - 46.0f, pos.y + kThumb * 0.5f};
        for (int ring = 0; ring < 5; ++ring)
        {
            float rr    = 16.0f + static_cast<float>(ring) * 15.0f;
            float spin  = (hovered ? time * 0.8f : 0.0f) + static_cast<float>(ring) * 0.9f;
            draw->PathArcTo(swirl, rr, spin, spin + 3.6f, 32);
            draw->PathStroke(U32(ImVec4{1, 1, 1, 0.10f - 0.015f * static_cast<float>(ring)}), 0, 2.0f);
        }
        draw->AddCircleFilled(swirl, 9.0f, U32(WithAlpha(Color::Background, 0.85f)), 24);
        draw->PopClipRect();
        {
            FontScope font(GetFonts().Bold, Size::Display);
            String    initials = Initials(recent.Name);
            draw->AddText({pos.x + 18.0f, pos.y + kThumb - ImGui::GetTextLineHeight() - 12.0f},
                          U32(ImVec4{1, 1, 1, exists ? 0.95f : 0.5f}), initials.Raw());
        }

        // Hover: a play button on the thumbnail.
        bool playHovered = false;
        if (hovered && exists)
        {
            ImVec2 center = {max.x - 30.0f, pos.y + 28.0f};
            ImVec2 mouse  = ImGui::GetMousePos();
            playHovered   = std::hypot(mouse.x - center.x, mouse.y - center.y) < 16.0f;
            draw->AddCircleFilled(center, 16.0f,
                                  U32(playHovered ? Color::AccentAlt : ImVec4{1, 1, 1, 0.92f}), 32);
            FontScope font(GetFonts().Regular, 13.0f);
            ImVec2    size = ImGui::CalcTextSize(Icon::Play);
            draw->AddText({center.x - size.x * 0.5f + 1.0f, center.y - size.y * 0.5f},
                          U32(Color::Background), Icon::Play);
            if (playHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) open = true;
        }

        // Text.
        float textX = pos.x + 16.0f, textW = width - 32.0f;
        float y     = pos.y + kThumb + 12.0f;
        {
            FontScope font(GetFonts().Bold, 16.0f);
            String    name = Ellipsize(recent.Name, textW);
            draw->AddText({textX, y}, U32(exists ? Color::Text : Color::Muted), name.Raw());
            y += ImGui::GetTextLineHeight() + 4.0f;
        }
        {
            FontScope font(GetFonts().Regular, 12.5f);
            String    location = EllipsizeStart(recent.Location.StrView(), textW);
            draw->AddText({textX, y}, U32(Color::Faint), location.Raw());

            // Footer: engine and when it was opened.
            const EngineInstall* engine = FindInstall(recent.Engine);
            String engineText = engine ? engine->DisplayName
                              : recent.Engine.Empty() ? String("Default engine")
                                                      : recent.Engine;
            String footer = exists ? Format("{} {}", Icon::Cube, engineText)
                                   : Format("{} Missing on disk", Icon::Warning);
            String when   = Format("{} {}", Icon::Clock, TimeAgo(recent.LastOpened));
            float  footY  = max.y - ImGui::GetTextLineHeight() - 14.0f;
            float  whenW  = ImGui::CalcTextSize(when.Raw()).x;
            String clipped = Ellipsize(footer, textW - whenW - 12.0f);
            draw->AddText({textX, footY}, U32(exists ? Color::Muted : Color::Danger), clipped.Raw());
            draw->AddText({max.x - 16.0f - whenW, footY}, U32(Color::Faint), when.Raw());
        }

        // Outline last, over the thumbnail edge.
        draw->AddRect(pos, max,
                      U32(hovered ? WithAlpha(Color::Accent, playHovered ? 0.5f : 0.85f)
                                  : Color::Border),
                      kRound, 0, hovered ? 1.5f : 1.0f);
        if (hovered && !playHovered)
            ImGui::SetTooltip("%s", String(recent.Location.StrView()).Raw());

        // Context menu.
        bool forget = false;
        String openWith;
        bool   openWithChosen = false;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8.0f, 8.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.0f, 6.0f});
        if (ImGui::BeginPopupContextItem("##tilemenu"))
        {
            if (ImGui::MenuItem(Format("{}  Open", Icon::Play).Raw(), nullptr, false, exists))
                open = true;
            if (ImGui::BeginMenu(Format("{}  Open with", Icon::Cube).Raw(),
                                 exists && !m_Installs.Empty()))
            {
                for (const auto& install : m_Installs)
                    if (ImGui::MenuItem(install.DisplayName.Raw()))
                    {
                        openWith       = install.Id;
                        openWithChosen = true;
                    }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(Format("{}  Show in file manager", Icon::FolderOpen).Raw(),
                                nullptr, false, exists))
                Reveal(recent.Location);
            if (ImGui::MenuItem(Format("{}  Copy path", Icon::Copy).Raw()))
                ImGui::SetClipboardText(String(recent.Location.StrView()).Raw());
            ImGui::Separator();
            if (ImGui::MenuItem(Format("{}  Remove from list", Icon::Close).Raw())) forget = true;
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopID();

        if (forget)
        {
            EraseIf(m_Settings.RecentProjects, [&](const RecentProject& entry)
                    { return entry.Location.StrView() == recent.Location.StrView(); });
            SaveSettings();
            Notify(Format("Removed {} from the list", recent.Name));
        }
        else if (openWithChosen) OpenProject(recent.Location, openWith);
        else if (open) OpenProject(recent.Location, recent.Engine);
    }

    void LauncherLayer::DrawNewProjectModal()
    {
        if (m_OpenNewProject)
        {
            ImGui::OpenPopup("##NewProject");
            m_OpenNewProject  = false;
            m_TemplatesEngine = String("\n"); // rescan: the engine may have changed on disk
            if (m_NewEngine.Empty()) m_NewEngine = m_Settings.DefaultEngine;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});
        ImGui::SetNextWindowSize({680.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {28.0f, 26.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.0f, 8.0f});
        ImGui::PushStyleColor(ImGuiCol_PopupBg, Color::Surface);
        bool open = ImGui::BeginPopupModal("##NewProject", nullptr,
                                           ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                                               | ImGuiWindowFlags_NoSavedSettings);
        ImGui::PopStyleColor();
        if (!open)
        {
            ImGui::PopStyleVar(2);
            return;
        }

        IconBadge(Icon::Rocket, 44.0f, Color::Accent, Color::AccentAlt);
        ImGui::SameLine(0, 14);
        ImGui::BeginGroup();
        {
            FontScope font(GetFonts().Bold, 20.0f);
            ImGui::TextUnformatted("Create a project");
        }
        Muted("Nexus lays out the files and opens it.");
        ImGui::EndGroup();
        ImGui::Dummy({0, 10});

        if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        FieldLabel("NAME");
        Input("##name", m_NewName, sizeof(m_NewName));
        ImGui::Dummy({0, 4});
        FieldLabel("LOCATION");
        FolderInput("##location", m_NewLocation, sizeof(m_NewLocation));
        ImGui::Dummy({0, 4});
        FieldLabel("ENGINE");
        ImGui::SetNextItemWidth(-1.0f);
        DrawEngineCombo("##newengine", m_NewEngine);
        RefreshTemplates();
        if (!m_Templates.Empty())
        {
            ImGui::Dummy({0, 4});
            FieldLabel("TEMPLATE");
            DrawTemplatePicker();
        }

        Path   directory = Path(StringView(m_NewLocation)) / Path(StringView(m_NewName));
        String problem   = m_NewName[0] == '\0' ? String("Give the project a name")
                                                : ValidateNewProjectDirectory(directory);
        if (problem.Empty() && !PickInstall(m_Installs, m_NewEngine, m_Settings))
            problem = "Install an engine first (Engines page)";

        ImGui::Dummy({0, 6});
        {
            FontScope font(GetFonts().Regular, Size::Small);
            if (problem.Empty())
                TextColored(Color::Faint, Format("{}  {}", Icon::Folder,
                                                 EllipsizeStart(directory.StrView(), 600.0f)));
            else TextColored(Color::Danger, Format("{}  {}", Icon::Warning, problem));
        }
        ImGui::Dummy({0, 10});

        float createW = ButtonWidth("Create and open", Icon::Rocket);
        float cancelW = ButtonWidth("Cancel");
        AlignRight(createW + cancelW + 10.0f);
        if (Button("Cancel", nullptr, ButtonKind::eGhost) || ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();
        ImGui::SameLine(0, 10);
        bool submit = ImGui::IsKeyPressed(ImGuiKey_Enter) && problem.Empty();
        if (Button("Create and open", Icon::Rocket, ButtonKind::ePrimary, {}, problem.Empty())
            || submit)
        {
            CreateProject();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
        ImGui::PopStyleVar(2);
    }

    void LauncherLayer::RefreshTemplates()
    {
        // Templates come from the engine that will create the project.
        const EngineInstall* engine = PickInstall(m_Installs, m_NewEngine, m_Settings);
        String               id     = engine ? engine->Id : String();
        if (id == m_TemplatesEngine) return;
        m_TemplatesEngine = id;
        m_Templates       = engine ? ScanTemplates(*engine) : Vector<ProjectTemplate>();

        // Keep the choice if this engine has it too, else its default.
        auto has = [&](StringView wanted)
        {
            for (const auto& entry : m_Templates)
                if (entry.Id.View() == wanted) return true;
            return false;
        };
        if (!has(m_NewTemplate))
            m_NewTemplate = has(kDefaultTemplate) ? String(kDefaultTemplate)
                          : m_Templates.Empty()   ? String()
                                                  : m_Templates.Front().Id;
    }

    void LauncherLayer::DrawTemplatePicker()
    {
        constexpr float kGap = 10.0f, kHeight = 84.0f;
        float           width = (ImGui::GetContentRegionAvail().x - kGap) * 0.5f;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {kGap, kGap});
        for (usize i = 0; i < m_Templates.Size(); ++i)
        {
            const auto& entry    = m_Templates[i];
            bool        selected = entry.Id == m_NewTemplate;
            ImGui::PushID(entry.Id.Raw());
            if (i % 2 == 1) ImGui::SameLine();

            ImVec2 pos = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##template", {width, kHeight})) m_NewTemplate = entry.Id;
            bool        hovered = ImGui::IsItemHovered();
            ImDrawList* draw    = ImGui::GetWindowDrawList();
            ImVec2      max     = {pos.x + width, pos.y + kHeight};
            if (selected)
                GradientRect(draw, pos, max, WithAlpha(Color::Accent, 0.20f),
                             WithAlpha(Color::AccentAlt, 0.08f), 12.0f);
            else
                draw->AddRectFilled(pos, max, U32(hovered ? Color::SurfaceHi : Color::Background),
                                    12.0f);
            draw->AddRect(pos, max,
                          U32(selected  ? Color::Accent
                              : hovered ? Color::Faint
                                        : Color::Border),
                          12.0f, 0, selected ? 1.5f : 1.0f);

            // Icon badge.
            constexpr float kBadge = 38.0f;
            ImVec2          badge  = {pos.x + 14.0f, pos.y + 14.0f};
            GradientRect(draw, badge, {badge.x + kBadge, badge.y + kBadge},
                         selected ? Color::Accent : Mix(Color::Accent, Color::Surface, 0.55f),
                         selected ? Color::AccentAlt : Mix(Color::AccentAlt, Color::Surface, 0.7f),
                         10.0f);
            {
                FontScope   font(GetFonts().Regular, 17.0f);
                const char* icon = Icon::Named(entry.Icon);
                ImVec2      size = ImGui::CalcTextSize(icon);
                draw->AddText({badge.x + (kBadge - size.x) * 0.5f, badge.y + (kBadge - size.y) * 0.5f},
                              IM_COL32_WHITE, icon);
            }

            // Name, category and a two-line description.
            float textX = badge.x + kBadge + 12.0f, textW = max.x - textX - 12.0f;
            float y     = pos.y + 12.0f;
            {
                FontScope font(GetFonts().Bold, 15.0f);
                String    name = Ellipsize(entry.Name, textW - 50.0f);
                draw->AddText({textX, y}, U32(Color::Text), name.Raw());
                if (!entry.Category.Empty())
                {
                    float     nameW = ImGui::CalcTextSize(name.Raw()).x;
                    FontScope small(GetFonts().Medium, 11.5f);
                    ImVec2    size  = ImGui::CalcTextSize(entry.Category.Raw());
                    ImVec2    pill  = {textX + nameW + 8.0f, y + 2.0f};
                    draw->AddRectFilled(pill, {pill.x + size.x + 12.0f, pill.y + size.y + 4.0f},
                                        U32(WithAlpha(Color::AccentAlt, 0.14f)), 99.0f);
                    draw->AddText({pill.x + 6.0f, pill.y + 2.0f}, U32(Color::AccentAlt),
                                  entry.Category.Raw());
                }
                y += ImGui::GetTextLineHeight() + 4.0f;
            }
            FontScope font(GetFonts().Regular, 12.5f);
            draw->PushClipRect(pos, {max.x, max.y - 8.0f}, true);
            draw->AddText(nullptr, 0.0f, {textX, y}, U32(Color::Muted), entry.Description.Raw(),
                          nullptr, textW);
            draw->PopClipRect();
            ImGui::PopID();
        }
        ImGui::PopStyleVar();
    }

    bool LauncherLayer::DrawEngineCombo(const char* id, String& engineId)
    {
        const EngineInstall* current = PickInstall(m_Installs, engineId, m_Settings);
        String preview = current ? Format("{}  {}", Icon::Cube, current->DisplayName)
                                 : String("No engine installed");
        bool   changed = false;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {12.0f, 10.0f});
        if (ImGui::BeginCombo(id, preview.Raw()))
        {
            for (const auto& install : m_Installs)
            {
                bool   selected = current && current->Id == install.Id;
                String text     = install.DisplayName;
                if (install.Id == m_Settings.DefaultEngine) text += "  (default)";
                if (ImGui::Selectable(text.Raw(), selected))
                {
                    engineId = install.Id;
                    changed  = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar();
        return changed;
    }

    // -- Engines ----------------------------------------------------------------

    void LauncherLayer::DrawEnginesPage()
    {
        float addWidth = ButtonWidth("Add local build", Icon::Wrench);
        DrawPageHeader("Engines", "Nexus and Vortex versions this launcher can open projects with",
                       addWidth);
        if (Button("Add local build", Icon::Wrench))
        {
            if (auto file = Vortex::Window::OpenFile({}, ExecutableDirectory()))
            {
                Path   executable = *file;
                String name       = Format("Local: {}", executable.ParentPath());
                EraseIf(m_Settings.LocalBuilds, [&](const LocalBuild& b) { return b.Name == name; });
                m_Settings.LocalBuilds.PushBack({name, executable, {}});
                SaveSettings();
                Rescan();
                Notify(Format("Added {}", name));
            }
        }
        Tooltip("A Nexus executable you built yourself");
        ImGui::Dummy({0, 22});

        if (const Release* update = AvailableUpdate();
            update && !(m_Job && m_Job->Target().Version == update->Version))
        {
            DrawUpdateBanner(*update);
            ImGui::Dummy({0, 18});
        }

        DrawInstalledEngines();
        ImGui::Dummy({0, 26});
        DrawAvailableReleases();
    }

    void LauncherLayer::DrawUpdateBanner(const Release& release)
    {
        constexpr float kHeight = 86.0f;
        ImVec2          pos     = ImGui::GetCursorScreenPos();
        float           width   = ImGui::GetContentRegionAvail().x;
        ImDrawList*     draw    = ImGui::GetWindowDrawList();
        GradientRect(draw, pos, {pos.x + width, pos.y + kHeight}, WithAlpha(Color::Accent, 0.30f),
                     WithAlpha(Color::AccentAlt, 0.14f), 14.0f);
        draw->AddRect(pos, {pos.x + width, pos.y + kHeight}, U32(WithAlpha(Color::Accent, 0.5f)),
                      14.0f);

        ImGui::SetCursorScreenPos({pos.x + 20.0f, pos.y + 20.0f});
        IconBadge(Icon::ArrowUp, 46.0f, Color::Accent, Color::AccentAlt);
        ImGui::SameLine(0, 16);
        ImGui::BeginGroup();
        {
            FontScope font(GetFonts().Bold, 18.0f);
            Text(Format("Vortex {} is available", release.Version));
        }
        StringView notes = release.Notes;
        if (usize line = notes.Find('\n'); line != StringView::NPos) notes = notes.Substr(0, line);
        Muted(notes.Empty() ? StringView("A newer stable release than any you have installed.")
                            : StringView(Ellipsize(notes, width - 320.0f)));
        ImGui::EndGroup();

        float installW = ButtonWidth("Install update", Icon::Download);
        ImGui::SetCursorScreenPos({pos.x + width - installW - 22.0f, pos.y + (kHeight - 38.0f) * 0.5f});
        if (Button("Install update", Icon::Download, ButtonKind::ePrimary, {}, !m_Job))
            StartInstall(release);
        ImGui::SetCursorScreenPos({pos.x, pos.y + kHeight});
        ImGui::Dummy({0, 0});
    }

    void LauncherLayer::DrawInstalledEngines()
    {
        SectionHeading("Installed", Format("{}", m_Installs.Size()));
        ImGui::Dummy({0, 4});
        if (m_Installs.Empty())
        {
            BeginCard("##noinstalls");
            TextColored(Color::Warning, Format("{}  Nothing installed yet", Icon::Info));
            Muted("Install a release below, or add a Nexus you built yourself.");
            EndCard();
            return;
        }

        BeginCard("##installs", Color::Surface, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.0f, 0.0f});
        for (usize i = 0; i < m_Installs.Size(); ++i)
        {
            const auto& install   = m_Installs[i];
            bool        isDefault = install.Id == m_Settings.DefaultEngine
                             || (m_Settings.DefaultEngine.Empty() && i == 0);
            bool        missing   = !Exists(install.Executable);
            ImGui::PushID(install.Id.Raw());

            constexpr float kRow = 70.0f;
            ImVec2          pos  = ImGui::GetCursorScreenPos();
            float           w    = ImGui::GetContentRegionAvail().x;
            ImGui::SetNextItemAllowOverlap();
            ImGui::InvisibleButton("##row", {w, kRow});
            bool        hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlappedByItem);
            ImDrawList* draw    = ImGui::GetWindowDrawList();
            if (hovered) draw->AddRectFilled(pos, {pos.x + w, pos.y + kRow}, U32(Color::SurfaceHi), 10.0f);
            if (i > 0)
                draw->AddLine({pos.x + 12, pos.y}, {pos.x + w - 12, pos.y}, U32(Color::Border));

            ImGui::SetCursorScreenPos({pos.x + 14.0f, pos.y + 13.0f});
            ImVec4 tint = install.Local ? Color::Faint : ChannelColor(install.Channel);
            IconBadge(install.Local ? Icon::Wrench : Icon::Cube, 44.0f, Mix(tint, Color::Accent, 0.4f),
                      Mix(tint, Color::AccentAlt, 0.2f));
            ImGui::SameLine(0, 14);
            ImGui::BeginGroup();
            {
                FontScope font(GetFonts().Bold, 16.0f);
                Text(install.DisplayName);
            }
            ImGui::SameLine(0, 10);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
            Pill(install.Local ? StringView("local") : StringView(install.Channel),
                 install.Local ? Color::Muted : ChannelColor(install.Channel));
            if (isDefault)
            {
                ImGui::SameLine(0, 6);
                Pill("default", Color::Warning, Icon::Star);
            }
            if (missing)
            {
                ImGui::SameLine(0, 6);
                Pill("missing", Color::Danger, Icon::Warning);
            }
            {
                FontScope font(GetFonts().Regular, 12.5f);
                TextColored(Color::Faint, EllipsizeStart(install.Executable.StrView(), w - 360.0f));
            }
            ImGui::EndGroup();

            // Actions, right-aligned.
            float setW   = isDefault ? 0.0f : ButtonWidth("Make default") + 6.0f;
            float actions = setW + 32.0f * 2 + 6.0f;
            ImGui::SetCursorScreenPos({pos.x + w - actions - 14.0f, pos.y + (kRow - 38.0f) * 0.5f});
            if (!isDefault)
            {
                if (Button("Make default", nullptr, ButtonKind::eGhost))
                {
                    m_Settings.DefaultEngine = install.Id;
                    m_Draft.DefaultEngine    = install.Id;
                    SaveSettings();
                    Notify(Format("{} is now the default engine", install.DisplayName));
                }
                ImGui::SameLine(0, 6);
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
            if (IconButton("reveal", Icon::FolderOpen, "Show in file manager")) Reveal(install.Executable);
            ImGui::SameLine(0, 6);
            if (IconButton("remove", Icon::Trash, install.Local ? "Forget this build" : "Uninstall",
                           Color::Muted))
                m_PendingRemoval = install.Id;

            ImGui::SetCursorScreenPos({pos.x, pos.y + kRow});
            ImGui::Dummy({0, 0});
            ImGui::PopID();
        }
        ImGui::PopStyleVar();
        EndCard();
    }

    void LauncherLayer::DrawRemoveEngineModal()
    {
        if (!m_PendingRemoval.Empty() && !ImGui::IsPopupOpen("##RemoveEngine"))
            ImGui::OpenPopup("##RemoveEngine");

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::SetNextWindowSize({460.0f, 0.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {26.0f, 24.0f});
        ImGui::PushStyleColor(ImGuiCol_PopupBg, Color::Surface);
        bool open = ImGui::BeginPopupModal("##RemoveEngine", nullptr,
                                           ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::PopStyleColor();
        if (!open)
        {
            ImGui::PopStyleVar();
            return;
        }

        const EngineInstall* install = FindInstall(m_PendingRemoval);
        if (!install)
        {
            m_PendingRemoval = String();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            ImGui::PopStyleVar();
            return;
        }

        IconBadge(Icon::Trash, 40.0f, Color::Danger, Mix(Color::Danger, Color::Warning, 0.4f));
        ImGui::SameLine(0, 14);
        ImGui::BeginGroup();
        {
            FontScope font(GetFonts().Bold, 18.0f);
            Text(install->Local ? Format("Forget {}?", install->DisplayName)
                                : Format("Uninstall {}?", install->DisplayName));
        }
        ImGui::PushTextWrapPos(400.0f);
        if (install->Local) Muted("Horizon stops listing it. The files stay where they are.");
        else Muted(Format("This deletes {}.", install->Root));
        ImGui::PopTextWrapPos();
        ImGui::EndGroup();
        ImGui::Dummy({0, 14});

        const char* verb    = install->Local ? "Forget" : "Uninstall";
        float       verbW   = ButtonWidth(verb, Icon::Trash);
        float       cancelW = ButtonWidth("Cancel");
        AlignRight(verbW + cancelW + 10.0f);
        if (Button("Cancel", nullptr, ButtonKind::eGhost) || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            m_PendingRemoval = String();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0, 10);
        if (Button(verb, Icon::Trash, ButtonKind::eDanger))
        {
            String error;
            String name = install->DisplayName;
            if (install->Local)
            {
                String id = install->Id;
                EraseIf(m_Settings.LocalBuilds, [&](const LocalBuild& b)
                        { return Format("local:{}", b.Name) == id; });
                Notify(Format("Forgot {}", name));
            }
            else if (RemoveInstall(*install, error)) Notify(Format("Uninstalled {}", name));
            else Notify(error, true);
            if (m_Settings.DefaultEngine == m_PendingRemoval) m_Settings.DefaultEngine = String();
            SaveSettings();
            m_PendingRemoval = String();
            Rescan();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
        ImGui::PopStyleVar();
    }

    void LauncherLayer::DrawAvailableReleases()
    {
        SectionHeading("Available", m_Settings.ManifestUrl);
        float toggleW = ImGui::CalcTextSize("Show previews").x + 40.0f;
        ImGui::SameLine();
        AlignRight(toggleW + 38.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        if (Checkbox("Show previews", &m_Settings.ShowPrereleases))
        {
            m_Draft.ShowPrereleases = m_Settings.ShowPrereleases;
            SaveSettings();
        }
        ImGui::SameLine(0, 6);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.0f);
        if (m_FetchState == FetchState::eLoading) Spinner(9.0f, static_cast<float>(ImGui::GetTime()));
        else if (IconButton("refresh", Icon::Refresh, "Check for releases (F5)")) FetchManifest();
        ImGui::Dummy({0, 4});

        switch (m_FetchState)
        {
            case FetchState::eIdle: return;
            case FetchState::eLoading:
                BeginCard("##loading");
                Spinner(9.0f, static_cast<float>(ImGui::GetTime()));
                ImGui::SameLine(0, 12);
                Muted("Checking for releases...");
                EndCard();
                return;
            case FetchState::eFailed:
                BeginCard("##fetchfailed", WithAlpha(Color::Danger, 0.08f));
                TextColored(Color::Danger, Format("{}  Can't reach the release server", Icon::Warning));
                ImGui::PushTextWrapPos(0.0f);
                Muted(m_FetchError);
                ImGui::PopTextWrapPos();
                ImGui::Dummy({0, 4});
                {
                    FontScope font(GetFonts().Regular, Size::Small);
                    TextColored(Color::Faint, "Running the test server?  python3 Horizon/Server/horizon_server.py serve");
                }
                ImGui::Dummy({0, 4});
                if (Button("Try again", Icon::Refresh)) FetchManifest();
                EndCard();
                return;
            case FetchState::eReady: break;
        }

        usize shown = 0;
        for (const auto& release : m_Manifest->Releases)
        {
            if (!m_Settings.ShowPrereleases && !release.IsStable()) continue;
            DrawReleaseCard(release);
            ImGui::Dummy({0, 10});
            ++shown;
        }
        if (shown == 0)
        {
            BeginCard("##noreleases");
            Muted(m_Manifest->Releases.Empty() ? "The manifest lists no releases yet."
                                               : "No stable releases. Turn on previews to see the rest.");
            EndCard();
        }
    }

    void LauncherLayer::DrawReleaseCard(const Release& release)
    {
        const String    platform  = HostPlatform();
        const Artifact* artifact  = release.ForPlatform(platform);
        bool            installed = IsInstalled(release.Version);
        bool            active    = m_Job && m_Job->Target().Version == release.Version;

        ImGui::PushID(release.Version.Raw());
        BeginCard("##release", active ? Mix(Color::Surface, Color::Accent, 0.06f) : Color::Surface);
        float width = ImGui::GetContentRegionAvail().x;

        ImGui::BeginGroup();
        {
            FontScope font(GetFonts().Bold, 19.0f);
            Text(release.Version);
        }
        ImGui::SameLine(0, 10);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        Pill(release.Channel, ChannelColor(release.Channel));
        if (!release.Date.Empty())
        {
            ImGui::SameLine(0, 10);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
            FontScope font(GetFonts().Regular, Size::Small);
            TextColored(Color::Faint, Format("{} {}", Icon::Clock, release.Date));
        }
        {
            FontScope font(GetFonts().Regular, Size::Small);
            String details = artifact ? Format("Nexus {}  ·  Vortex {}  ·  {}", release.NexusVersion,
                                               release.VortexVersion, FormatSize(artifact->Size))
                                      : Format("{}  No build for {}", Icon::Warning, platform);
            TextColored(artifact ? Color::Muted : Color::Warning, details);
        }
        ImGui::EndGroup();

        // Action, right side.
        float actionW = active ? 280.0f : installed ? 110.0f : ButtonWidth("Install", Icon::Download);
        ImVec2 groupMin = ImGui::GetItemRectMin();
        ImGui::SameLine();
        ImGui::SetCursorScreenPos({groupMin.x + width - actionW, groupMin.y + 6.0f});
        if (active) DrawInstallProgress(actionW);
        else if (installed)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 110.0f - 96.0f);
            Pill("Installed", Color::Success, Icon::Check);
        }
        else if (Button("Install", Icon::Download, ButtonKind::ePrimary, {}, artifact && !m_Job))
            StartInstall(release);

        if (!release.Notes.Empty())
        {
            ImGui::Dummy({0, 2});
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width - (active ? 300.0f : 140.0f));
            Muted(release.Notes);
            ImGui::PopTextWrapPos();
        }
        EndCard();
        ImGui::PopID();
    }

    void LauncherLayer::DrawInstallProgress(float width)
    {
        ImGui::BeginGroup();
        const char* what = "Downloading";
        switch (m_Job->CurrentState())
        {
            case InstallJob::State::eVerifying: what = "Verifying checksum"; break;
            case InstallJob::State::eExtracting: what = "Unpacking"; break;
            default: break;
        }
        float  progress = m_Job->Progress();
        String status   = m_Job->CurrentState() == InstallJob::State::eDownloading
                            ? Format("{}  {} of {}  ·  {}/s", what, FormatSize(m_Job->BytesReceived()),
                                     FormatSize(m_Job->BytesTotal()),
                                     FormatSize(static_cast<u64>(m_BytesPerSecond)))
                            : String(what);
        {
            FontScope font(GetFonts().Regular, 12.5f);
            TextColored(Color::AccentAlt, Ellipsize(status, width - 40.0f));
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        bool indeterminate = m_Job->CurrentState() != InstallJob::State::eDownloading;
        Progress(indeterminate ? -1.0f : progress, {width - 40.0f, 8.0f},
                 static_cast<float>(ImGui::GetTime()));
        ImGui::EndGroup();
        ImGui::SameLine(0, 8);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        if (IconButton("cancel", Icon::Close, "Cancel the download")) m_Job->Cancel();
    }

    // -- Settings -----------------------------------------------------------------

    void LauncherLayer::DrawSettingsPage()
    {
        bool  dirty   = DraftChanged();
        float saveW   = ButtonWidth("Save", Icon::Check);
        float revertW = ButtonWidth("Revert");
        DrawPageHeader("Settings", Settings::FilePath().StrView(), saveW + revertW + 10.0f);
        if (Button("Revert", nullptr, ButtonKind::eGhost, {}, dirty)) LoadDraft();
        ImGui::SameLine(0, 10);
        if (Button("Save", Icon::Check, ButtonKind::ePrimary, {}, dirty)
            || (dirty && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)))
            ApplyDraft();
        ImGui::Dummy({0, 22});

        auto section = [](const char* id, const char* icon, const char* title, const char* caption)
        {
            BeginCard(id, Color::Surface, 22.0f);
            IconBadge(icon, 34.0f, Mix(Color::Accent, Color::Surface, 0.3f),
                      Mix(Color::AccentAlt, Color::Surface, 0.5f));
            ImGui::SameLine(0, 12);
            ImGui::BeginGroup();
            {
                FontScope font(GetFonts().Bold, 16.0f);
                ImGui::TextUnformatted(title);
            }
            {
                FontScope font(GetFonts().Regular, Size::Small);
                Muted(caption);
            }
            ImGui::EndGroup();
            ImGui::Dummy({0, 8});
        };

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {12.0f, 10.0f});
        section("##downloads", Icon::Download, "Downloads", "Where releases come from and where they go");
        FieldLabel("RELEASE MANIFEST URL", "http(s):// or file://");
        Input("##manifest", m_DraftManifestUrl, sizeof(m_DraftManifestUrl));
        ImGui::Dummy({0, 4});
        FieldLabel("INSTALL ENGINES IN");
        FolderInput("##installdir", m_DraftInstallDir, sizeof(m_DraftInstallDir));
        ImGui::Dummy({0, 4});
        Checkbox("Show nightly and preview releases", &m_Draft.ShowPrereleases);
        EndCard();
        ImGui::Dummy({0, 14});

        section("##projects", Icon::Folder, "Projects", "Defaults for new projects");
        FieldLabel("NEW PROJECTS GO IN");
        FolderInput("##projectsdir", m_DraftProjectsDir, sizeof(m_DraftProjectsDir));
        ImGui::Dummy({0, 4});
        Checkbox("Close Horizon after opening a project", &m_Draft.CloseAfterLaunch);
        EndCard();
        ImGui::Dummy({0, 14});

        section("##editor", Icon::Code, "Editor", "How Nexus is started");
        FieldLabel("DEFAULT ENGINE");
        ImGui::SetNextItemWidth(-1.0f);
        DrawEngineCombo("##defaultengine", m_Draft.DefaultEngine);
        ImGui::Dummy({0, 4});
        FieldLabel("EXTRA NEXUS ARGUMENTS", "split on spaces");
        Input("##extraargs", m_DraftExtraArgs, sizeof(m_DraftExtraArgs), "--verbose");
        EndCard();
        ImGui::PopStyleVar();
        ImGui::Dummy({0, 14});

        section("##about", Icon::Info, "About", "Where Horizon keeps things");
        auto row = [](const char* label, StringView value)
        {
            FontScope font(GetFonts().Regular, Size::Small);
            TextColored(Color::Faint, label);
            ImGui::SameLine(170.0f);
            Text(value);
        };
        row("Settings file", Settings::FilePath().StrView());
        row("Engines", m_Settings.InstallDirectory.StrView());
        row("Editor logs", (DataDirectory() / "logs").StrView());
        row("Platform", HostPlatform());
        EndCard();
    }

    // -- Toasts -------------------------------------------------------------------

    void LauncherLayer::DrawToasts()
    {
        double now = ImGui::GetTime();
        EraseIf(m_Toasts, [&](const Toast& toast)
                { return now - toast.Shown > (toast.Error ? 9.0 : 4.5); });
        if (m_Toasts.Empty()) return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImDrawList*          draw     = ImGui::GetForegroundDrawList(const_cast<ImGuiViewport*>(viewport));
        float                y        = viewport->WorkPos.y + viewport->WorkSize.y - 24.0f;
        FontScope            font(GetFonts().Medium, Size::Body);
        for (usize i = m_Toasts.Size(); i-- > 0;)
        {
            const Toast& toast    = m_Toasts[i];
            double       age      = now - toast.Shown;
            double       lifetime = toast.Error ? 9.0 : 4.5;
            float        alpha    = static_cast<float>(age < 0.2 ? age / 0.2
                                                       : age > lifetime - 0.4 ? (lifetime - age) / 0.4
                                                                              : 1.0);
            float slide = static_cast<float>(age < 0.2 ? (1.0 - age / 0.2) * 24.0 : 0.0);

            String text  = Format("{}   {}", toast.Error ? Icon::Warning : Icon::Check, toast.Text);
            text         = Ellipsize(text, 520.0f);
            ImVec2 size  = ImGui::CalcTextSize(text.Raw());
            ImVec2 pad   = {18.0f, 13.0f};
            float  w     = size.x + pad.x * 2, h = size.y + pad.y * 2;
            float  x     = viewport->WorkPos.x + viewport->WorkSize.x - w - 24.0f + slide;
            ImVec2 min   = {x, y - h}, max = {x + w, y};
            ImVec4 tint  = toast.Error ? Color::Danger : Color::Success;
            draw->AddRectFilled({min.x + 2, min.y + 6}, {max.x + 2, max.y + 8},
                                U32(ImVec4{0, 0, 0, 0.35f * alpha}), 12.0f);
            draw->AddRectFilled(min, max, U32(WithAlpha(Color::SurfaceHi, alpha)), 12.0f);
            draw->AddRect(min, max, U32(WithAlpha(tint, 0.55f * alpha)), 12.0f);
            draw->AddRectFilled({min.x, min.y + 10}, {min.x + 3, max.y - 10},
                                U32(WithAlpha(tint, alpha)), 2.0f);
            draw->AddText({min.x + pad.x, min.y + pad.y}, U32(WithAlpha(Color::Text, alpha)),
                          text.Raw());
            y -= h + 10.0f;
        }
    }

    // -- Actions --------------------------------------------------------------------

    void LauncherLayer::Rescan() { m_Installs = ScanInstalls(m_Settings); }

    void LauncherLayer::FetchManifest()
    {
        // Called with m_FetchMutex held (from the UI); the previous worker has
        // already published (it's only joinable here once it's finishing).
        if (m_FetchState == FetchState::eLoading) return;
        if (m_FetchThread.joinable()) m_FetchThread.join();

        m_FetchState  = FetchState::eLoading;
        m_FetchThread = std::thread(
            [this, url = m_Settings.ManifestUrl]
            {
                String             body, error;
                Optional<Manifest> manifest;
                auto               result = Http::Get(url, body);
                if (!result.Ok) error = Format("Can't download the manifest: {}", result.Error);
                else manifest = ParseManifest(body, url, error);

                std::lock_guard lock(m_FetchMutex);
                m_Manifest   = Move(manifest);
                m_FetchError = error;
                m_FetchState = m_Manifest ? FetchState::eReady : FetchState::eFailed;
            });
    }

    void LauncherLayer::StartInstall(const Release& release)
    {
        const Artifact* artifact = release.ForPlatform(HostPlatform());
        if (!artifact || m_Job) return;
        m_Job            = CreateScope<InstallJob>(release, *artifact, m_Settings.InstallDirectory);
        m_SpeedBytes     = 0;
        m_SpeedTime      = Clock::now();
        m_BytesPerSecond = 0.0;
        Notify(Format("Installing Vortex {}", release.Version));
    }

    void LauncherLayer::LoadDraft()
    {
        m_Draft = m_Settings;
        CopyTo(m_DraftManifestUrl, sizeof(m_DraftManifestUrl), m_Settings.ManifestUrl);
        CopyTo(m_DraftInstallDir, sizeof(m_DraftInstallDir), m_Settings.InstallDirectory.StrView());
        CopyTo(m_DraftProjectsDir, sizeof(m_DraftProjectsDir), m_Settings.ProjectsDirectory.StrView());
        CopyTo(m_DraftExtraArgs, sizeof(m_DraftExtraArgs), m_Settings.ExtraArguments);
    }

    bool LauncherLayer::DraftChanged() const
    {
        return m_Settings.ManifestUrl.View() != StringView(m_DraftManifestUrl)
            || m_Settings.InstallDirectory.StrView() != StringView(m_DraftInstallDir)
            || m_Settings.ProjectsDirectory.StrView() != StringView(m_DraftProjectsDir)
            || m_Settings.ExtraArguments.View() != StringView(m_DraftExtraArgs)
            || m_Settings.DefaultEngine != m_Draft.DefaultEngine
            || m_Settings.CloseAfterLaunch != m_Draft.CloseAfterLaunch
            || m_Settings.ShowPrereleases != m_Draft.ShowPrereleases;
    }

    void LauncherLayer::ApplyDraft()
    {
        bool manifestChanged = m_Settings.ManifestUrl.View() != StringView(m_DraftManifestUrl);
        bool installsChanged = m_Settings.InstallDirectory.StrView() != StringView(m_DraftInstallDir);
        m_Settings.ManifestUrl       = String(StringView(m_DraftManifestUrl));
        m_Settings.InstallDirectory  = Path(StringView(m_DraftInstallDir));
        m_Settings.ProjectsDirectory = Path(StringView(m_DraftProjectsDir));
        m_Settings.ExtraArguments    = String(StringView(m_DraftExtraArgs));
        m_Settings.DefaultEngine     = m_Draft.DefaultEngine;
        m_Settings.CloseAfterLaunch  = m_Draft.CloseAfterLaunch;
        m_Settings.ShowPrereleases   = m_Draft.ShowPrereleases;
        String error;
        if (!m_Settings.Save(&error)) return Notify(error, true);
        if (installsChanged) Rescan();
        if (manifestChanged) FetchManifest();
        LoadDraft();
        Notify("Settings saved");
    }

    void LauncherLayer::SaveSettings()
    {
        String error;
        if (!m_Settings.Save(&error)) Notify(error, true);
    }

    void LauncherLayer::BrowseForProject()
    {
        auto filters = Prism::ToArray<Vortex::Window::FileFilter>({{"Vortex Project file", "vproj"}});
        if (auto file = Vortex::Window::OpenFile(filters, m_Settings.ProjectsDirectory))
            OpenProject(*file, {});
    }

    void LauncherLayer::OpenProject(const Path& path, StringView engineId)
    {
        auto project = ReadProject(path);
        if (!project) return Notify(Format("Not a Vortex project: {}", path), true);

        const EngineInstall* engine = PickInstall(m_Installs, engineId, m_Settings);
        if (!engine) return Notify("Install an engine first (Engines page)", true);

        if (auto error = OpenInEditor(*engine, *project, m_Settings); !error.Empty())
            return Notify(error, true);

        // Entries are keyed by the .vproj; a directory entry is replaced.
        RememberProject(m_Settings, project->Name, project->File,
                        engineId.Empty() ? StringView() : StringView(engine->Id));
        SaveSettings();
        Notify(Format("Opening {} in {}", project->Name, engine->DisplayName));
        AfterLaunch();
    }

    void LauncherLayer::CreateProject()
    {
        const EngineInstall* engine = PickInstall(m_Installs, m_NewEngine, m_Settings);
        if (!engine) return;
        Path directory = Path(StringView(m_NewLocation)) / Path(StringView(m_NewName));
        if (auto error = CreateInEditor(*engine, directory, m_NewName, m_NewTemplate, m_Settings);
            !error.Empty())
            return Notify(error, true);

        // Nexus names the .vproj; the directory stands in until it's opened
        // again from here.
        bool explicitEngine = !m_NewEngine.Empty() && m_NewEngine != m_Settings.DefaultEngine;
        RememberProject(m_Settings, m_NewName, directory,
                        explicitEngine ? StringView(engine->Id) : StringView());
        SaveSettings();
        Notify(Format("Creating {} in {}", StringView(m_NewName), engine->DisplayName));
        AfterLaunch();
    }

    void LauncherLayer::AfterLaunch()
    {
        if (m_Settings.CloseAfterLaunch && !m_Job) Vortex::Application::Get()->Close();
    }

    void LauncherLayer::Notify(String message, bool isError)
    {
        if (message.Empty()) return;
        m_Toasts.PushBack({Move(message), isError, ImGui::GetTime()});
        if (m_Toasts.Size() > 4) m_Toasts.Erase(m_Toasts.begin());
    }

    const EngineInstall* LauncherLayer::FindInstall(StringView id) const
    {
        for (const auto& install : m_Installs)
            if (install.Id.View() == id) return &install;
        return nullptr;
    }

    bool LauncherLayer::IsInstalled(StringView version) const
    {
        for (const auto& install : m_Installs)
            if (!install.Local && install.Version.View() == version) return true;
        return false;
    }

    const Release* LauncherLayer::AvailableUpdate() const
    {
        if (m_FetchState != FetchState::eReady || !m_Manifest) return nullptr;
        const String platform = HostPlatform();
        for (const auto& release : m_Manifest->Releases)
        {
            if (!release.IsStable() || !release.ForPlatform(platform)) continue;
            // Releases are newest first: the first stable one is the latest.
            for (const auto& install : m_Installs)
                if (!install.Local && CompareVersions(install.Version, release.Version) >= 0)
                    return nullptr;
            return &release;
        }
        return nullptr;
    }
} // namespace Horizon
