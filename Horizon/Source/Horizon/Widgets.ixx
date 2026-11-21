/*
 * Created by v1tr10l7 on 21.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <imgui.h>

#include <cmath>
#include <cstdio>
#include <initializer_list>

export module Horizon.Widgets;

import Prism.String.Formatter;
import Horizon.Platform;

/*
 * Horizon's look: an "event horizon" theme (deep space background, a
 * violet-to-cyan accent), its fonts and icons, and the custom widgets the
 * launcher pages are built from.
 */
export namespace Horizon::Ui
{
    // -- Palette ---------------------------------------------------------------

    namespace Color
    {
        constexpr ImVec4 Background = {0.043f, 0.051f, 0.075f, 1.0f}; // #0B0D13
        constexpr ImVec4 Sidebar    = {0.059f, 0.067f, 0.098f, 1.0f}; // #0F1119
        constexpr ImVec4 Surface    = {0.082f, 0.094f, 0.133f, 1.0f}; // #151822
        constexpr ImVec4 SurfaceHi  = {0.110f, 0.125f, 0.176f, 1.0f}; // #1C202D
        constexpr ImVec4 Border     = {0.165f, 0.184f, 0.247f, 1.0f}; // #2A2F3F
        constexpr ImVec4 Text       = {0.910f, 0.925f, 0.957f, 1.0f}; // #E8ECF4
        constexpr ImVec4 Muted      = {0.533f, 0.565f, 0.651f, 1.0f}; // #8890A6
        constexpr ImVec4 Faint      = {0.345f, 0.369f, 0.443f, 1.0f}; // #585E71
        constexpr ImVec4 Accent     = {0.486f, 0.361f, 1.000f, 1.0f}; // #7C5CFF
        constexpr ImVec4 AccentAlt  = {0.212f, 0.761f, 1.000f, 1.0f}; // #36C2FF
        constexpr ImVec4 Success    = {0.243f, 0.812f, 0.557f, 1.0f}; // #3ECF8E
        constexpr ImVec4 Warning    = {0.961f, 0.722f, 0.255f, 1.0f}; // #F5B841
        constexpr ImVec4 Danger     = {0.957f, 0.373f, 0.408f, 1.0f}; // #F45F68
    } // namespace Color

    inline ImVec4 WithAlpha(ImVec4 c, float a) { return {c.x, c.y, c.z, c.w * a}; }
    inline ImVec4 Mix(ImVec4 a, ImVec4 b, float t)
    {
        return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t};
    }
    inline ImU32 U32(ImVec4 c) { return ImGui::ColorConvertFloat4ToU32(c); }

    /// A rounded rect filled with a left-to-right gradient.
    void GradientRect(ImDrawList* draw, ImVec2 min, ImVec2 max, ImVec4 left, ImVec4 right,
                      float rounding, ImDrawFlags flags = 0)
    {
        // ImGui's own gradient shader keeps each vertex's alpha, so this
        // interpolates all four channels itself.
        int first = draw->VtxBuffer.Size;
        draw->AddRectFilled(min, max, IM_COL32_WHITE, rounding, flags);
        float width = max.x - min.x > 0.0f ? max.x - min.x : 1.0f;
        for (int i = first; i < draw->VtxBuffer.Size; ++i)
        {
            ImDrawVert& vertex = draw->VtxBuffer[i];
            float       t      = (vertex.pos.x - min.x) / width;
            t                  = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
            // Anti-aliased fringe vertices carry their own coverage alpha.
            float       aa     = static_cast<float>((vertex.col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
            ImVec4      color  = Mix(left, right, t);
            color.w *= aa;
            vertex.col = U32(color);
        }
    }

    /// A stable, pleasant colour for a name (project thumbnails).
    ImVec4       ColorFor(StringView text)
    {
        u32 hash = 2166136261u;
        for (char c : text) hash = (hash ^ static_cast<u8>(c)) * 16777619u;
        float h = static_cast<float>(hash % 360) / 360.0f, r, g, b;
        ImGui::ColorConvertHSVtoRGB(h, 0.55f, 0.85f, r, g, b);
        return {r, g, b, 1.0f};
    }

    // -- Icons (Font Awesome 4 codepoints, from the bundled Nerd Font) --------

    namespace Icon
    {
        constexpr const char* Folder      = "\uf07b";
        constexpr const char* FolderOpen  = "\uf07c";
        constexpr const char* Plus        = "\uf067";
        constexpr const char* Cube        = "\uf1b2";
        constexpr const char* Cubes       = "\uf1b3";
        constexpr const char* Cog         = "\uf013";
        constexpr const char* Download    = "\uf019";
        constexpr const char* Search      = "\uf002";
        constexpr const char* Star        = "\uf005";
        constexpr const char* Trash       = "\uf1f8";
        constexpr const char* Play        = "\uf04b";
        constexpr const char* Rocket      = "\uf135";
        constexpr const char* Check       = "\uf00c";
        constexpr const char* Close       = "\uf00d";
        constexpr const char* Clock       = "\uf017";
        constexpr const char* Refresh     = "\uf021";
        constexpr const char* Warning     = "\uf071";
        constexpr const char* Info        = "\uf05a";
        constexpr const char* Hdd         = "\uf0a0";
        constexpr const char* Tag         = "\uf02b";
        constexpr const char* Code        = "\uf121";
        constexpr const char* Ellipsis    = "\uf141";
        constexpr const char* ArrowUp     = "\uf062";
        constexpr const char* Copy        = "\uf0c5";
        constexpr const char* Wrench      = "\uf0ad";
        constexpr const char* Globe       = "\uf0ac";
        constexpr const char* Bolt        = "\uf0e7";
        constexpr const char* File        = "\uf15b";
        constexpr const char* Gamepad     = "\uf11b";
        constexpr const char* Image       = "\uf03e";
        constexpr const char* Leaf        = "\uf06c";

        /// An icon by the names project templates use ("cube", "bolt"...).
        const char* Named(StringView name)
        {
            if (name == "file") return File;
            if (name == "bolt") return Bolt;
            if (name == "rocket") return Rocket;
            if (name == "gamepad") return Gamepad;
            if (name == "globe") return Globe;
            if (name == "code") return Code;
            if (name == "image") return Image;
            if (name == "leaf") return Leaf;
            if (name == "star") return Star;
            return Cube;
        }
    } // namespace Icon

    // -- Fonts -----------------------------------------------------------------

    struct Fonts
    {
        ImFont* Regular = nullptr;
        ImFont* Medium  = nullptr;
        ImFont* Bold    = nullptr;
    };

    Fonts& GetFonts()
    {
        static Fonts fonts;
        return fonts;
    }

    namespace Size
    {
        constexpr float Body    = 15.0f;
        constexpr float Small   = 13.0f;
        constexpr float Title   = 26.0f;
        constexpr float Heading = 17.0f;
        constexpr float Display = 34.0f;
    } // namespace Size

    /// Adds the UI fonts (a system sans when there is one, else the engine's
    /// bundled font) with the icon font merged into each. ImGui 1.92 bakes
    /// glyphs on demand, so any size works later with PushFont(font, size).
    void LoadFonts()
    {
        ImGuiIO& io        = ImGui::GetIO();

        auto     firstOf   = [](std::initializer_list<const char*> candidates) -> const char*
        {
            for (const char* path : candidates)
                if (Exists(path)) return path;
            return nullptr;
        };
        const char* regular = firstOf({"/usr/share/fonts/noto/NotoSans-Regular.ttf",
                                       "/usr/share/fonts/TTF/NotoSans-Regular.ttf",
                                       "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
                                       "C:/Windows/Fonts/segoeui.ttf",
                                       "assets/fonts/ComicMono.ttf"});
        const char* medium  = firstOf({"/usr/share/fonts/noto/NotoSans-Medium.ttf",
                                       "/usr/share/fonts/TTF/NotoSans-Medium.ttf",
                                       "/usr/share/fonts/truetype/noto/NotoSans-Medium.ttf",
                                       "C:/Windows/Fonts/seguisb.ttf"});
        const char* bold    = firstOf({"/usr/share/fonts/noto/NotoSans-Bold.ttf",
                                       "/usr/share/fonts/TTF/NotoSans-Bold.ttf",
                                       "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf",
                                       "C:/Windows/Fonts/segoeuib.ttf",
                                       "assets/fonts/ComicMono-Bold.ttf"});
        const char* icons   = firstOf({"assets/fonts/JetBrainsMonoNerdFont-SemiBoldItalic.ttf"});

        auto        add     = [&](const char* path) -> ImFont*
        {
            if (!path) return nullptr;
            ImFont* font = io.Fonts->AddFontFromFileTTF(path, Size::Body);
            if (font && icons)
            {
                ImFontConfig merge;
                merge.MergeMode        = true;
                merge.GlyphOffset      = {0.0f, 1.0f};
                merge.GlyphMinAdvanceX = Size::Body;
                io.Fonts->AddFontFromFileTTF(icons, Size::Body, &merge);
            }
            return font;
        };

        Fonts& fonts  = GetFonts();
        fonts.Regular = add(regular);
        fonts.Medium  = medium ? add(medium) : fonts.Regular;
        fonts.Bold    = bold ? add(bold) : fonts.Medium;
        if (!fonts.Regular) fonts.Regular = io.FontDefault;
        if (!fonts.Medium) fonts.Medium = fonts.Regular;
        if (!fonts.Bold) fonts.Bold = fonts.Medium;
        io.FontDefault = fonts.Regular;
    }

    /// RAII PushFont/PopFont.
    struct FontScope
    {
        FontScope(ImFont* font, float size) { ImGui::PushFont(font, size); }
        ~FontScope() { ImGui::PopFont(); }
        FontScope(const FontScope&)            = delete;
        FontScope& operator=(const FontScope&) = delete;
    };

    // -- Theme -----------------------------------------------------------------

    void ApplyTheme()
    {
        ImGuiStyle& style              = ImGui::GetStyle();
        style.WindowPadding            = {0.0f, 0.0f};
        style.FramePadding             = {12.0f, 8.0f};
        style.ItemSpacing              = {10.0f, 10.0f};
        style.ItemInnerSpacing         = {8.0f, 6.0f};
        style.ScrollbarSize            = 10.0f;
        style.GrabMinSize              = 10.0f;
        style.WindowBorderSize         = 0.0f;
        style.ChildBorderSize          = 1.0f;
        style.PopupBorderSize          = 1.0f;
        style.FrameBorderSize          = 1.0f;
        style.WindowRounding           = 12.0f;
        style.ChildRounding            = 12.0f;
        style.FrameRounding            = 8.0f;
        style.PopupRounding            = 10.0f;
        style.ScrollbarRounding        = 8.0f;
        style.GrabRounding             = 8.0f;
        style.TabRounding              = 8.0f;
        style.SeparatorTextBorderSize  = 1.0f;

        ImVec4* c                      = style.Colors;
        c[ImGuiCol_Text]               = Color::Text;
        c[ImGuiCol_TextDisabled]       = Color::Muted;
        c[ImGuiCol_WindowBg]           = Color::Background;
        c[ImGuiCol_ChildBg]            = {0, 0, 0, 0};
        c[ImGuiCol_PopupBg]            = Color::SurfaceHi;
        c[ImGuiCol_Border]             = Color::Border;
        c[ImGuiCol_BorderShadow]       = {0, 0, 0, 0};
        c[ImGuiCol_FrameBg]            = Color::Surface;
        c[ImGuiCol_FrameBgHovered]     = Color::SurfaceHi;
        c[ImGuiCol_FrameBgActive]      = Mix(Color::SurfaceHi, Color::Accent, 0.15f);
        c[ImGuiCol_TitleBg]            = Color::Sidebar;
        c[ImGuiCol_TitleBgActive]      = Color::Sidebar;
        c[ImGuiCol_ScrollbarBg]        = {0, 0, 0, 0};
        c[ImGuiCol_ScrollbarGrab]      = Color::Border;
        c[ImGuiCol_ScrollbarGrabHovered] = Color::Faint;
        c[ImGuiCol_ScrollbarGrabActive]  = Color::Muted;
        c[ImGuiCol_CheckMark]          = Color::Accent;
        c[ImGuiCol_SliderGrab]         = Color::Accent;
        c[ImGuiCol_SliderGrabActive]   = Color::AccentAlt;
        c[ImGuiCol_Button]             = Color::Surface;
        c[ImGuiCol_ButtonHovered]      = Color::SurfaceHi;
        c[ImGuiCol_ButtonActive]       = Mix(Color::SurfaceHi, Color::Accent, 0.25f);
        c[ImGuiCol_Header]             = WithAlpha(Color::Accent, 0.20f);
        c[ImGuiCol_HeaderHovered]      = WithAlpha(Color::Accent, 0.28f);
        c[ImGuiCol_HeaderActive]       = WithAlpha(Color::Accent, 0.36f);
        c[ImGuiCol_Separator]          = Color::Border;
        c[ImGuiCol_SeparatorHovered]   = Color::Accent;
        c[ImGuiCol_SeparatorActive]    = Color::AccentAlt;
        c[ImGuiCol_TextSelectedBg]     = WithAlpha(Color::Accent, 0.35f);
        c[ImGuiCol_NavCursor]          = Color::Accent;
        c[ImGuiCol_ModalWindowDimBg]   = {0.0f, 0.0f, 0.0f, 0.60f};
        c[ImGuiCol_PlotHistogram]      = Color::Accent;
    }

    // -- Small helpers -----------------------------------------------------------

    /// `text`, shortened with "..." to fit `width`.
    String Ellipsize(StringView text, float width)
    {
        String full(text);
        if (ImGui::CalcTextSize(full.Raw()).x <= width) return full;
        constexpr const char* kDots = "...";
        float dots = ImGui::CalcTextSize(kDots).x;
        usize keep = text.Size();
        while (keep > 0)
        {
            --keep;
            String candidate(text.Substr(0, keep));
            if (ImGui::CalcTextSize(candidate.Raw()).x + dots <= width)
            {
                candidate += kDots;
                return candidate;
            }
        }
        return String(kDots);
    }

    /// The same, keeping the end ("...dev/Game/Game.vproj") for paths.
    String EllipsizeStart(StringView text, float width)
    {
        String full(text);
        if (ImGui::CalcTextSize(full.Raw()).x <= width) return full;
        constexpr const char* kDots = "...";
        float dots = ImGui::CalcTextSize(kDots).x;
        for (usize skip = 1; skip < text.Size(); ++skip)
        {
            String tail(text.Substr(skip));
            if (ImGui::CalcTextSize(tail.Raw()).x + dots <= width)
            {
                String result(kDots);
                result += tail;
                return result;
            }
        }
        return String(kDots);
    }

    String FormatSize(u64 bytes)
    {
        char text[32];
        if (bytes >= 1024ull * 1024 * 1024)
            std::snprintf(text, sizeof(text), "%.2f GB", static_cast<double>(bytes) / (1024.0 * 1024 * 1024));
        else if (bytes >= 1024 * 1024)
            std::snprintf(text, sizeof(text), "%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024));
        else std::snprintf(text, sizeof(text), "%.0f KB", static_cast<double>(bytes) / 1024.0);
        return String(text);
    }

    void TextColored(ImVec4 color, StringView text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(text.Raw(), text.Raw() + text.Size());
        ImGui::PopStyleColor();
    }
    void Text(StringView text) { ImGui::TextUnformatted(text.Raw(), text.Raw() + text.Size()); }
    void Muted(StringView text) { TextColored(Color::Muted, text); }

    void Tooltip(const char* text)
    {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) ImGui::SetTooltip("%s", text);
    }

    // -- Logo --------------------------------------------------------------------

    /// The Horizon mark: a black hole with a glowing, slowly turning
    /// accretion ring.
    void Logo(ImDrawList* draw, ImVec2 center, float radius, float time)
    {
        constexpr int kSegments = 64;
        // Soft outer glow.
        for (int i = 6; i >= 1; --i)
        {
            float t = static_cast<float>(i) / 6.0f;
            draw->AddCircleFilled(center, radius * (1.0f + 0.45f * t),
                                  U32(WithAlpha(Color::Accent, 0.05f * (1.0f - t) + 0.01f)),
                                  kSegments);
        }
        // Accretion ring: arcs whose colour sweeps from violet to cyan.
        constexpr int kArcs = 48;
        for (int i = 0; i < kArcs; ++i)
        {
            float a0    = time * 0.6f + static_cast<float>(i) * (6.2831853f / kArcs);
            float a1    = a0 + 6.2831853f / kArcs + 0.02f;
            float t     = 0.5f + 0.5f * std::sin(a0 * 1.0f);
            ImVec4 col  = Mix(Color::Accent, Color::AccentAlt, t);
            float  glow = 0.55f + 0.45f * std::sin(a0 * 2.0f + time);
            draw->PathArcTo(center, radius * 0.80f, a0, a1, 6);
            draw->PathStroke(U32(WithAlpha(col, glow)), 0, radius * 0.22f);
        }
        // The event horizon itself.
        draw->AddCircleFilled(center, radius * 0.58f, U32(Color::Background), kSegments);
        draw->AddCircle(center, radius * 0.58f, U32(WithAlpha(Color::AccentAlt, 0.55f)),
                        kSegments, 1.5f);
    }

    // -- Buttons -----------------------------------------------------------------

    enum class ButtonKind
    {
        ePrimary,
        eSecondary,
        eGhost,
        eDanger,
    };

    /// A rounded button with an optional leading icon.
    bool Button(const char* label, const char* icon = nullptr, ButtonKind kind = ButtonKind::eSecondary,
                ImVec2 size = {0, 0}, bool enabled = true)
    {
        ImGui::PushID(label);
        Fonts& fonts   = GetFonts();
        String text    = icon ? Format("{}  {}", icon, label) : String(label);
        // Labels after "##" are ids only.
        if (usize hash = text.Find("##"); hash != StringView::NPos) text = String(text.View().Substr(0, hash));

        ImGui::PushFont(fonts.Medium, Size::Body);
        ImVec2 textSize = ImGui::CalcTextSize(text.Raw());
        ImVec2 pad      = {16.0f, 9.0f};
        ImVec2 extent   = {size.x > 0 ? size.x : textSize.x + pad.x * 2,
                           size.y > 0 ? size.y : textSize.y + pad.y * 2};
        if (size.x < 0) extent.x = ImGui::GetContentRegionAvail().x;

        ImGui::BeginDisabled(!enabled);
        ImVec2 pos     = ImGui::GetCursorScreenPos();
        bool   pressed = ImGui::InvisibleButton("##button", extent);
        bool   hovered = ImGui::IsItemHovered();
        bool   held    = ImGui::IsItemActive();
        ImGui::EndDisabled();

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2      max  = {pos.x + extent.x, pos.y + extent.y};
        float       a    = enabled ? 1.0f : 0.45f;
        ImVec4      fg   = Color::Text;
        switch (kind)
        {
            case ButtonKind::ePrimary:
            {
                ImVec4 left  = Mix(Color::Accent, Color::Text, hovered ? 0.10f : 0.0f);
                ImVec4 right = Mix(Color::AccentAlt, Color::Text, hovered ? 0.10f : 0.0f);
                if (held)
                {
                    left  = Mix(left, Color::Background, 0.15f);
                    right = Mix(right, Color::Background, 0.15f);
                }
                GradientRect(draw, pos, max, WithAlpha(left, a), WithAlpha(right, a), 8.0f);
                fg = {1, 1, 1, 1};
                break;
            }
            case ButtonKind::eSecondary:
                draw->AddRectFilled(pos, max,
                                    U32(WithAlpha(hovered ? Color::SurfaceHi : Color::Surface, a)),
                                    8.0f);
                draw->AddRect(pos, max, U32(WithAlpha(hovered ? Color::Faint : Color::Border, a)),
                              8.0f);
                break;
            case ButtonKind::eGhost:
                if (hovered) draw->AddRectFilled(pos, max, U32(Color::Surface), 8.0f);
                fg = hovered ? Color::Text : Color::Muted;
                break;
            case ButtonKind::eDanger:
                draw->AddRectFilled(pos, max,
                                    U32(WithAlpha(Color::Danger, (hovered ? 0.28f : 0.16f) * a)),
                                    8.0f);
                fg = Color::Danger;
                break;
        }
        draw->AddText({pos.x + (extent.x - textSize.x) * 0.5f, pos.y + (extent.y - textSize.y) * 0.5f},
                      U32(WithAlpha(fg, a)), text.Raw());
        ImGui::PopFont();
        ImGui::PopID();
        return pressed && enabled;
    }

    /// A square, icon-only ghost button.
    bool IconButton(const char* id, const char* icon, const char* tooltip = nullptr,
                    ImVec4 color = Color::Muted, float size = 32.0f)
    {
        ImGui::PushID(id);
        ImVec2 pos     = ImGui::GetCursorScreenPos();
        bool   pressed = ImGui::InvisibleButton("##icon", {size, size});
        bool   hovered = ImGui::IsItemHovered();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        if (hovered)
            draw->AddRectFilled(pos, {pos.x + size, pos.y + size}, U32(Color::SurfaceHi), 8.0f);
        ImVec2 textSize = ImGui::CalcTextSize(icon);
        draw->AddText({pos.x + (size - textSize.x) * 0.5f, pos.y + (size - textSize.y) * 0.5f},
                      U32(hovered ? Color::Text : color), icon);
        if (tooltip) Tooltip(tooltip);
        ImGui::PopID();
        return pressed;
    }

    /// A small rounded label ("stable", "nightly", "default").
    void Pill(StringView text, ImVec4 color, const char* icon = nullptr)
    {
        Fonts& fonts = GetFonts();
        FontScope font(fonts.Medium, Size::Small);
        String label = icon ? Format("{} {}", icon, text) : String(text);
        ImVec2 size  = ImGui::CalcTextSize(label.Raw());
        ImVec2 pad   = {8.0f, 3.0f};
        ImVec2 pos   = ImGui::GetCursorScreenPos();
        ImVec2 max   = {pos.x + size.x + pad.x * 2, pos.y + size.y + pad.y * 2};
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(pos, max, U32(WithAlpha(color, 0.16f)), 99.0f);
        draw->AddText({pos.x + pad.x, pos.y + pad.y}, U32(color), label.Raw());
        ImGui::Dummy({max.x - pos.x, max.y - pos.y});
    }

    ImVec4 ChannelColor(StringView channel)
    {
        if (channel == "stable") return Color::Success;
        if (channel == "local") return Color::Muted;
        if (channel == "nightly") return Color::Accent;
        return Color::Warning;
    }

    /// A rounded progress bar with an accent gradient; a moving stripe
    /// when `fraction` is negative (size unknown).
    void Progress(float fraction, ImVec2 size, float time)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        if (size.x <= 0) size.x = ImGui::GetContentRegionAvail().x;
        ImVec2      max  = {pos.x + size.x, pos.y + size.y};
        ImDrawList* draw = ImGui::GetWindowDrawList();
        float       r    = size.y * 0.5f;
        draw->AddRectFilled(pos, max, U32(Color::SurfaceHi), r);
        if (fraction >= 0)
        {
            float w = size.x * (fraction > 1 ? 1 : fraction);
            if (w > size.y) GradientRect(draw, pos, {pos.x + w, max.y}, Color::Accent, Color::AccentAlt, r);
        }
        else
        {
            float span  = size.x * 0.3f;
            float t     = std::fmod(time * 0.8f, 1.0f);
            float start = pos.x - span + (size.x + span) * t;
            draw->PushClipRect(pos, max, true);
            draw->AddRectFilledMultiColor({start, pos.y}, {start + span, max.y},
                                          U32(WithAlpha(Color::Accent, 0.0f)), U32(Color::AccentAlt),
                                          U32(Color::AccentAlt), U32(WithAlpha(Color::Accent, 0.0f)));
            draw->PopClipRect();
        }
        ImGui::Dummy(size);
    }

    /// A spinning arc, for "checking..." states.
    void Spinner(float radius, float time, ImVec4 color = Color::AccentAlt)
    {
        ImVec2 pos    = ImGui::GetCursorScreenPos();
        ImVec2 center = {pos.x + radius, pos.y + radius + 2.0f};
        float  start  = time * 6.0f;
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->PathArcTo(center, radius, start, start + 4.2f, 24);
        draw->PathStroke(U32(color), 0, 2.5f);
        ImGui::Dummy({radius * 2, radius * 2 + 4.0f});
    }

    // -- Cards -------------------------------------------------------------------

    /// Starts a padded, rounded panel that grows to fit its contents.
    void BeginCard(const char* id, ImVec4 background = Color::Surface, float padding = 18.0f)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, background);
        ImGui::PushStyleColor(ImGuiCol_Border, Color::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {padding, padding});
        ImGui::BeginChild(id, {0, 0},
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY
                              | ImGuiChildFlags_AlwaysUseWindowPadding,
                          ImGuiWindowFlags_NoScrollbar);
    }
    void EndCard()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }

    /// A section heading with an optional muted caption on its right.
    void SectionHeading(const char* title, StringView caption = {})
    {
        {
            FontScope font(GetFonts().Bold, Size::Heading);
            ImGui::TextUnformatted(title);
        }
        if (!caption.Empty())
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
            Muted(caption);
        }
    }

    /// A checkbox sized for body text, whatever the frame padding is.
    bool Checkbox(const char* label, bool* value)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {3.0f, 3.0f});
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        bool changed = ImGui::Checkbox(label, value);
        ImGui::PopStyleVar(2);
        return changed;
    }

    /// Label above an input that fills the width.
    void FieldLabel(const char* label, const char* hint = nullptr)
    {
        FontScope font(GetFonts().Medium, Size::Small);
        TextColored(Color::Muted, label);
        if (hint)
        {
            ImGui::SameLine();
            TextColored(Color::Faint, hint);
        }
    }
} // namespace Horizon::Ui
