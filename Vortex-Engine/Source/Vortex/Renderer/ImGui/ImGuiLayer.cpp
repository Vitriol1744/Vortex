/*
 * Created by v1tr10l7 on 10.07.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Time.hpp>
#include <Vortex/Renderer/ImGui/ImGuiLayer.hpp>

import Prism.String.StringUtils;

#include <ImGuizmo.h>

namespace Vortex
{
    using Input::KeyCode;
    using Input::MouseCode;

    inline void SetupImGuiStyle(bool bStyleDark_, float alpha_)
    {
        ImGuiStyle& style           = ImGui::GetStyle();

        // light style from Pacôme Danhiez (user itamago)
        // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
        style.Alpha                 = 1.0f;
        style.FrameRounding         = 3.0f;
        style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]
            = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        style.Colors[ImGuiCol_DockingEmptyBg]
            = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_Border]  = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
        style.Colors[ImGuiCol_BorderShadow]
            = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_FrameBgHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]
            = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
        style.Colors[ImGuiCol_TitleBgActive]
            = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg]   = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab]
            = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]
            = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]
            = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        // style.Colors[ImGuiCol_ComboBg]    = ImVec4(0.86f, 0.86f, 0.86f,
        // 0.99f);
        style.Colors[ImGuiCol_CheckMark]  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive]
            = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_Col] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        // style.Colors[ImGuiCol_ColumnHovered]
        //     = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        // style.Colors[ImGuiCol_ColumnActive]
        //     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        style.Colors[ImGuiCol_ResizeGripHovered]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        // style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f,
        // 0.50f); style.Colors[ImGuiCol_CloseButtonHovered]
        //     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        // style.Colors[ImGuiCol_CloseButtonActive]
        //     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]
            = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]
            = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]
            = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]
            = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        // style.Colors[ImGuiCol_ModalWindowDarkening]
        //     = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

        if (bStyleDark_)
        {
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];
                float   H, S, V;
                ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

                if (S < 0.1f) { V = 1.0f - V; }
                ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
                if (col.w < 1.00f) { col.w *= alpha_; }
            }
        }
        else {
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                ImVec4& col = style.Colors[i];
                if (col.w < 1.00f)
                {
                    col.x *= alpha_;
                    col.y *= alpha_;
                    col.z *= alpha_;
                    col.w *= alpha_;
                }
            }
        }
    }
    inline void SetupDarkImGuiStyle()
{
    auto& style  = ImGui::GetStyle();
    auto& colors = style.Colors;

    // Dark charcoal palette with high contrast
    constexpr auto bgBase        = IM_COL32(15, 15, 17, 255);   // Panel backgrounds
    constexpr auto bgDark        = IM_COL32(10, 10, 12, 255);   // Deep gaps & borders
    constexpr auto titlebarColor = IM_COL32(20, 20, 23, 255);   // Titlebars & menu
    constexpr auto inputBg       = IM_COL32(24, 24, 28, 255);   // Input fields & headers
    constexpr auto textColor     = IM_COL32(220, 220, 225, 255); // High legibility text
    constexpr auto accentHover   = IM_COL32(39, 185, 242, 255); // Highlight cyan

    // Window & Panel Backgrounds
    colors[ImGuiCol_WindowBg]         = ImGui::ColorConvertU32ToFloat4(bgBase);
    colors[ImGuiCol_ChildBg]          = ImGui::ColorConvertU32ToFloat4(bgBase);
    colors[ImGuiCol_PopupBg]          = ImGui::ColorConvertU32ToFloat4(bgBase);
    colors[ImGuiCol_DockingEmptyBg]   = ImGui::ColorConvertU32ToFloat4(bgDark);

    // Headers & Tree Nodes
    colors[ImGuiCol_Header]           = ImGui::ColorConvertU32ToFloat4(inputBg);
    colors[ImGuiCol_HeaderHovered]    = ImColor(45, 45, 52, 255);
    colors[ImGuiCol_HeaderActive]     = ImColor(55, 55, 65, 255);

    // Titlebar & Menus
    colors[ImGuiCol_TitleBg]          = ImGui::ColorConvertU32ToFloat4(titlebarColor);
    colors[ImGuiCol_TitleBgActive]    = ImGui::ColorConvertU32ToFloat4(titlebarColor);
    colors[ImGuiCol_TitleBgCollapsed] = ImGui::ColorConvertU32ToFloat4(bgDark);
    colors[ImGuiCol_MenuBarBg]        = ImGui::ColorConvertU32ToFloat4(titlebarColor);

    // Tabs
    colors[ImGuiCol_Tab]                = ImGui::ColorConvertU32ToFloat4(titlebarColor);
    colors[ImGuiCol_TabHovered]         = ImColor(40, 40, 48, 255);
    colors[ImGuiCol_TabActive]          = ImGui::ColorConvertU32ToFloat4(bgBase);
    colors[ImGuiCol_TabUnfocused]       = ImGui::ColorConvertU32ToFloat4(titlebarColor);
    colors[ImGuiCol_TabUnfocusedActive] = ImGui::ColorConvertU32ToFloat4(bgBase);

    // Widgets (Inputs, Buttons, Sliders)
    colors[ImGuiCol_FrameBg]        = ImGui::ColorConvertU32ToFloat4(inputBg);
    colors[ImGuiCol_FrameBgHovered] = ImColor(35, 35, 42, 255);
    colors[ImGuiCol_FrameBgActive]  = ImColor(45, 45, 55, 255);

    colors[ImGuiCol_Button]        = ImColor(30, 30, 36, 255);
    colors[ImGuiCol_ButtonHovered] = ImColor(45, 45, 55, 255);
    colors[ImGuiCol_ButtonActive]  = ImColor(60, 60, 72, 255);

    // Borders & Separators
    colors[ImGuiCol_Border]           = ImGui::ColorConvertU32ToFloat4(bgDark);
    colors[ImGuiCol_BorderShadow]     = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_Separator]        = ImGui::ColorConvertU32ToFloat4(bgDark);
    colors[ImGuiCol_SeparatorHovered] = ImGui::ColorConvertU32ToFloat4(accentHover);
    colors[ImGuiCol_SeparatorActive]  = ImGui::ColorConvertU32ToFloat4(accentHover);

    // Text & Accents
    colors[ImGuiCol_Text]          = ImGui::ColorConvertU32ToFloat4(textColor);
    colors[ImGuiCol_TextDisabled]  = ImColor(100, 100, 108, 255);
    colors[ImGuiCol_CheckMark]     = ImGui::ColorConvertU32ToFloat4(accentHover);
    colors[ImGuiCol_SliderGrab]    = ImColor(60, 60, 70, 255);
    colors[ImGuiCol_SliderGrabActive] = ImGui::ColorConvertU32ToFloat4(accentHover);

    // Styling Adjustments
    style.FrameRounding = 3.0f;
    style.WindowRounding = 0.0f;
    style.TabRounding = 3.0f;
    style.FrameBorderSize = 1.0f;
}    ImGui_ImplVortex_Data* GetBackendData()
    {
        return reinterpret_cast<ImGui_ImplVortex_Data*>(
            ImGui::GetIO().BackendPlatformUserData);
    }
    inline static ImGuiViewportData* GetViewportData(ImGuiViewport* viewport)
    {
        return reinterpret_cast<ImGuiViewportData*>(viewport->PlatformUserData);
    }
    ImGuiKey ToImGuiKey(KeyCode key)
    {
        using Input::KeyCode;

        switch (key)
        {
            case KeyCode::eTab: return ImGuiKey_Tab;
            case KeyCode::eLeft: return ImGuiKey_LeftArrow;
            case KeyCode::eRight: return ImGuiKey_RightArrow;
            case KeyCode::eUp: return ImGuiKey_UpArrow;
            case KeyCode::eDown: return ImGuiKey_DownArrow;
            case KeyCode::ePageUp: return ImGuiKey_PageUp;
            case KeyCode::ePageDown: return ImGuiKey_PageDown;
            case KeyCode::eHome: return ImGuiKey_Home;
            case KeyCode::eEnd: return ImGuiKey_End;
            case KeyCode::eInsert: return ImGuiKey_Insert;
            case KeyCode::eDelete: return ImGuiKey_Delete;
            case KeyCode::eBackspace: return ImGuiKey_Backspace;
            case KeyCode::eSpace: return ImGuiKey_Space;
            case KeyCode::eReturn: return ImGuiKey_Enter;
            case KeyCode::eEscape: return ImGuiKey_Escape;
            case KeyCode::eApostrophe: return ImGuiKey_Apostrophe;
            case KeyCode::eComma: return ImGuiKey_Comma;
            case KeyCode::eHyphen: return ImGuiKey_Minus;
            case KeyCode::ePeriod: return ImGuiKey_Period;
            case KeyCode::eSlash: return ImGuiKey_Slash;
            case KeyCode::eSemicolon: return ImGuiKey_Semicolon;
            case KeyCode::eEqual: return ImGuiKey_Equal;
            case KeyCode::eLBracket: return ImGuiKey_LeftBracket;
            case KeyCode::eBackslash: return ImGuiKey_Backslash;
            case KeyCode::eRBracket: return ImGuiKey_RightBracket;
            case KeyCode::eTilde: return ImGuiKey_GraveAccent;
            case KeyCode::eCapsLock: return ImGuiKey_CapsLock;
            case KeyCode::eScrollLock: return ImGuiKey_ScrollLock;
            case KeyCode::eNumLock: return ImGuiKey_NumLock;
            case KeyCode::ePrintScreen: return ImGuiKey_PrintScreen;
            case KeyCode::ePause: return ImGuiKey_Pause;
            case KeyCode::eNumpad0: return ImGuiKey_Keypad0;
            case KeyCode::eNumpad1: return ImGuiKey_Keypad1;
            case KeyCode::eNumpad2: return ImGuiKey_Keypad2;
            case KeyCode::eNumpad3: return ImGuiKey_Keypad3;
            case KeyCode::eNumpad4: return ImGuiKey_Keypad4;
            case KeyCode::eNumpad5: return ImGuiKey_Keypad5;
            case KeyCode::eNumpad6: return ImGuiKey_Keypad6;
            case KeyCode::eNumpad7: return ImGuiKey_Keypad7;
            case KeyCode::eNumpad8: return ImGuiKey_Keypad8;
            case KeyCode::eNumpad9: return ImGuiKey_Keypad9;
            case KeyCode::eDecimal: return ImGuiKey_KeypadDecimal;
            case KeyCode::eDivide: return ImGuiKey_KeypadDivide;
            case KeyCode::eMultiply: return ImGuiKey_KeypadMultiply;
            case KeyCode::eSubtract: return ImGuiKey_KeypadSubtract;
            case KeyCode::eAdd: return ImGuiKey_KeypadAdd;
            case KeyCode::eEnter: return ImGuiKey_KeypadEnter;
            // case KeyCode: return ImGuiKey_KeypadEqual;
            case KeyCode::eLShift: return ImGuiKey_LeftShift;
            case KeyCode::eLCtrl: return ImGuiKey_LeftCtrl;
            case KeyCode::eLAlt: return ImGuiKey_LeftAlt;
            case KeyCode::eLSystem: return ImGuiKey_LeftSuper;
            case KeyCode::eRShift: return ImGuiKey_RightShift;
            case KeyCode::eRCtrl: return ImGuiKey_RightCtrl;
            case KeyCode::eRAlt: return ImGuiKey_RightAlt;
            case KeyCode::eRSystem: return ImGuiKey_RightSuper;
            case KeyCode::eMenu: return ImGuiKey_Menu;
            case KeyCode::eNum0: return ImGuiKey_0;
            case KeyCode::eNum1: return ImGuiKey_1;
            case KeyCode::eNum2: return ImGuiKey_2;
            case KeyCode::eNum3: return ImGuiKey_3;
            case KeyCode::eNum4: return ImGuiKey_4;
            case KeyCode::eNum5: return ImGuiKey_5;
            case KeyCode::eNum6: return ImGuiKey_6;
            case KeyCode::eNum7: return ImGuiKey_7;
            case KeyCode::eNum8: return ImGuiKey_8;
            case KeyCode::eNum9: return ImGuiKey_9;
            case KeyCode::eA: return ImGuiKey_A;
            case KeyCode::eB: return ImGuiKey_B;
            case KeyCode::eC: return ImGuiKey_C;
            case KeyCode::eD: return ImGuiKey_D;
            case KeyCode::eE: return ImGuiKey_E;
            case KeyCode::eF: return ImGuiKey_F;
            case KeyCode::eG: return ImGuiKey_G;
            case KeyCode::eH: return ImGuiKey_H;
            case KeyCode::eI: return ImGuiKey_I;
            case KeyCode::eJ: return ImGuiKey_J;
            case KeyCode::eK: return ImGuiKey_K;
            case KeyCode::eL: return ImGuiKey_L;
            case KeyCode::eM: return ImGuiKey_M;
            case KeyCode::eN: return ImGuiKey_N;
            case KeyCode::eO: return ImGuiKey_O;
            case KeyCode::eP: return ImGuiKey_P;
            case KeyCode::eQ: return ImGuiKey_Q;
            case KeyCode::eR: return ImGuiKey_R;
            case KeyCode::eS: return ImGuiKey_S;
            case KeyCode::eT: return ImGuiKey_T;
            case KeyCode::eU: return ImGuiKey_U;
            case KeyCode::eV: return ImGuiKey_V;
            case KeyCode::eW: return ImGuiKey_W;
            case KeyCode::eX: return ImGuiKey_X;
            case KeyCode::eY: return ImGuiKey_Y;
            case KeyCode::eZ: return ImGuiKey_Z;
            case KeyCode::eF1: return ImGuiKey_F1;
            case KeyCode::eF2: return ImGuiKey_F2;
            case KeyCode::eF3: return ImGuiKey_F3;
            case KeyCode::eF4: return ImGuiKey_F4;
            case KeyCode::eF5: return ImGuiKey_F5;
            case KeyCode::eF6: return ImGuiKey_F6;
            case KeyCode::eF7: return ImGuiKey_F7;
            case KeyCode::eF8: return ImGuiKey_F8;
            case KeyCode::eF9: return ImGuiKey_F9;
            case KeyCode::eF10: return ImGuiKey_F10;
            case KeyCode::eF11: return ImGuiKey_F11;
            case KeyCode::eF12: return ImGuiKey_F12;
            case KeyCode::eF13: return ImGuiKey_F13;
            case KeyCode::eF14: return ImGuiKey_F14;
            case KeyCode::eF15: return ImGuiKey_F15;
            /*case KeyCode::eF16: return ImGuiKey_F16;
            case KeyCode::eF17: return ImGuiKey_F17;
            case KeyCode::eF18: return ImGuiKey_F18;
            case KeyCode::eF19: return ImGuiKey_F19;
            case KeyCode::eF20: return ImGuiKey_F20;
            case KeyCode::eF21: return ImGuiKey_F21;
            case KeyCode::eF22: return ImGuiKey_F22;
            case KeyCode::eF23: return ImGuiKey_F23;
            case KeyCode::eF24: return ImGuiKey_F24;*/
            default: break;
        }

        return ImGuiKey_None;
    }
    ImGuiMouseButton ToImGuiMouseButton(MouseCode button)
    {
        switch (button)
        {
            case MouseCode::eLeft: return ImGuiMouseButton_Left;
            case MouseCode::eRight: return ImGuiMouseButton_Right;
            case MouseCode::eMiddle: return ImGuiMouseButton_Middle;

            default: break;
        }

        return ImGuiMouseButton_COUNT;
    }
    void UpdateKeyModifiers(KeyCode key, bool pressed)
    {
        ImGuiIO& io = ImGui::GetIO();

        switch (key)
        {
            case KeyCode::eLCtrl:
            case KeyCode::eRCtrl: io.AddKeyEvent(ImGuiMod_Ctrl, pressed); break;
            case KeyCode::eLAlt:
            case KeyCode::eRAlt: io.AddKeyEvent(ImGuiMod_Alt, pressed); break;
            case KeyCode::eLShift:
            case KeyCode::eRShift:
                io.AddKeyEvent(ImGuiMod_Shift, pressed);
                break;
            case KeyCode::eLSystem:
            case KeyCode::eRSystem:
                io.AddKeyEvent(ImGuiMod_Super, pressed);
                break;

            default: break;
        }
    }

    void ImGuiLayer::OnAttach()
    {
        auto& window    = m_MainWindow;
        auto  swapChain = window.SwapChain();

        IMGUI_CHECKVERSION();
        VtCoreTrace("ImGui: Creating context...");
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        VtCoreAssert(io.BackendPlatformUserData == nullptr
                     && "Already initialized a platform backend!");

        io.ConfigFlags
            |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags
            |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        // Platform windows. On Wayland they're popups of the main window
        // (see CreateWindow), in a virtual coordinate space.
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        f32 fontSize   = 16.8f;
        io.FontDefault = io.Fonts->AddFontFromFileTTF(
            "assets/fonts/ComicMono-Bold.ttf", fontSize);
        io.Fonts->AddFontFromFileTTF("assets/fonts/ComicMono.ttf", fontSize);
        io.Fonts->AddFontFromFileTTF(
            "assets/fonts/JetBrainsMonoNerdFont-SemiBoldItalic.ttf", fontSize);
        // SetupImGuiStyle(true, 0.94f);
        SetupDarkImGuiStyle();

        // When viewports are enabled we tweak WindowRounding/WindowBg so
        // platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        //
        // Setup Platform/Renderer backends
        // Setup backend capabilities flags
        ImGui_ImplVortex_Data* bd  = new ImGui_ImplVortex_Data;
        io.BackendPlatformUserData = reinterpret_cast<void*>(bd);
        io.BackendPlatformName     = "Vortex-Engine";
        // io.BackendFlags
        //     |= ImGuiBackendFlags_HasMouseCursors; // We can honor
        //  GetMouseCursor() values
        //  (optional)
        io.BackendFlags
            |= ImGuiBackendFlags_HasSetMousePos; // We can honor
                                                 // io.WantSetMousePos requests
                                                 // (optional, rarely used)

        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
        // Wayland: pointer focus stays on the surface a drag started in
        // (implicit grab), and NoInputs viewports aren't skipped, so ImGui
        // finds the hovered viewport from positions instead.
        if (Window::HasGlobalPositions())
            io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;

        bd->MainWindow         = &window;
        bd->Time               = 0.0;
        bd->WantUpdateMonitors = true;

        // TODO(v1tr10l7): Clipboard
        io.SetClipboardTextFn  = [](void*, const char*) {};
        io.GetClipboardTextFn  = [](void*) -> const char* { return ""; };
        io.ClipboardUserData   = &window;
        CreateCursors();
        SetUpEvents();
        UpdateMonitors();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            InitializePlatformInterface();

        // Set platform dependent data in viewport
        ImGuiViewport* mainViewport    = ImGui::GetMainViewport();
        auto           data            = new ImGuiViewportData;
        data->Window                   = &bd->MainWindow->Window();
        data->WindowOwned              = false;
        mainViewport->PlatformUserData = data;
        // A Window*, like every other viewport's (FindViewportByPlatformHandle
        // and the cursor code rely on it).
        mainViewport->PlatformHandle = data->Window;

        m_Backend = CreateScope<VortexBackend>();
    }
    void ImGuiLayer::OnDetach()
    {
        VtCoreTrace("Detaching ImGuiLayer...");

        auto mainViewport = ImGui::GetMainViewport();
        delete reinterpret_cast<ImGuiViewportData*>(
            mainViewport->PlatformUserData);
        mainViewport->PlatformUserData = nullptr;
        mainViewport->PlatformHandle   = nullptr;

        ImGui_ImplVortex_Data* bd = GetBackendData();
        VtCoreAssert(
            bd != nullptr
            && "No platform backend to shutdown, or already shutdown?");
        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            VtCoreTrace("ImGuiLayer: Destroying platform windows...");
            ImGui::DestroyPlatformWindows();
        }
        // After the platform windows: their swapchains belong to the backend.
        m_Backend.Reset();
        io.BackendPlatformName     = nullptr;
        io.BackendPlatformUserData = nullptr;
        io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors
                             | ImGuiBackendFlags_HasSetMousePos
                             | ImGuiBackendFlags_HasGamepad
                             | ImGuiBackendFlags_PlatformHasViewports
                             | ImGuiBackendFlags_HasMouseHoveredViewport);
        delete bd;
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin()
    {
        VtProfileFunction();

        ImGuiIO& io              = ImGui::GetIO();
        auto     bd              = GetBackendData();
        auto     windowSize      = bd->MainWindow->Size();

        auto     framebufferSize = bd->MainWindow->FramebufferSize();
        io.DisplaySize           = ImVec2(static_cast<f32>(windowSize.x),
                                          static_cast<f32>(windowSize.y));
        if (windowSize.x > 0 && windowSize.y > 0)
            io.DisplayFramebufferScale
                = ImVec2(static_cast<f32>(framebufferSize.x)
                             / static_cast<f32>(windowSize.x),
                         static_cast<f32>(framebufferSize.y)
                             / static_cast<f32>(windowSize.y));
        if (bd->WantUpdateMonitors) UpdateMonitors();

        f64 currentTime = Time::CurrentTime();
        if (currentTime <= bd->Time) currentTime = bd->Time + 0.00001f;
        io.DeltaTime = bd->Time > 0.0 ? (f32)(currentTime - bd->Time)
                                      : (f32)(1.0f / 60.0f);
        bd->Time     = currentTime;
        UpdateMouseData();
        UpdateMouseCursor();

        // TODO(v1tr10l7): Gamepad support for imgui
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }
    void ImGuiLayer::End()
    {
        VtProfileFunction();
        ImGui::Render();
        m_Backend->RenderViewports(m_MainWindow);
    }

    void ImGuiLayer::OnUpdate(Timestep deltaTime) {}
    void ImGuiLayer::OnRender() {}
    void ImGuiLayer::OnImGuiRender() {}

    void ImGuiLayer::CreateCursors()
    {
        // TODO(v1tr10l7): ImGuiLayer::CreateCursors()
    }
    void ImGuiLayer::SetUpEvents()
    {
        m_EventBus.Subscribe<KeyPressedEvent>(OnKeyPressed);
        m_EventBus.Subscribe<KeyReleasedEvent>(OnKeyReleased);
        m_EventBus.Subscribe<KeyTypedEvent>(OnKeyTyped);

        m_EventBus.Subscribe<MouseButtonPressedEvent>(OnMouseButtonPressed);
        m_EventBus.Subscribe<MouseButtonReleasedEvent>(OnMouseButtonReleased);
        m_EventBus.Subscribe<MouseScrolledEvent>(OnMouseScrolled);
        m_EventBus.Subscribe<MouseMovedEvent>(OnMouseMoved);
        m_EventBus.Subscribe<MouseEnteredEvent>(OnMouseEntered);
        m_EventBus.Subscribe<MouseLeftEvent>(OnMouseLeft);

        m_EventBus.Subscribe<WindowMovedEvent>(OnWindowMoved);
        m_EventBus.Subscribe<WindowResizedEvent>(OnWindowResized);
        m_EventBus.Subscribe<WindowFocusedEvent>(OnWindowFocusGained);
        m_EventBus.Subscribe<WindowFocusLostEvent>(OnWindowFocusLost);
        m_EventBus.Subscribe<WindowClosedEvent>(OnWindowClosed);

        m_EventBus.Subscribe<MonitorStateChangedEvent>(OnMonitorStateUpdate);
    }
    void ImGuiLayer::InitializePlatformInterface()
    {
        ImGuiPlatformIO& io            = ImGui::GetPlatformIO();
        io.Platform_CreateWindow       = CreateWindow;
        io.Platform_DestroyWindow      = DestroyWindow;
        io.Platform_ShowWindow         = ShowWindow;
        io.Platform_SetWindowPos       = SetWindowPosition;
        io.Platform_GetWindowPos       = WindowPosition;
        io.Platform_SetWindowSize      = SetWindowSize;
        io.Platform_GetWindowSize      = WindowSize;
        io.Platform_SetWindowFocus     = RequestWindowFocus;
        io.Platform_GetWindowFocus     = IsWindowFocused;
        io.Platform_GetWindowMinimized = IsWindowMinimized;
        io.Platform_SetWindowTitle     = SetWindowTitle;
        io.Platform_SetWindowAlpha     = SetWindowOpacity;
        io.Platform_RenderWindow       = [](ImGuiViewport*, void*) {};
        io.Platform_SwapBuffers        = [](ImGuiViewport*, void*) {};
    }

    void ImGuiLayer::UpdateMouseData()
    {
        ImGui_ImplVortex_Data* bd              = GetBackendData();
        ImGuiIO&               io              = ImGui::GetIO();
        ImGuiPlatformIO&       platformIO      = ImGui::GetPlatformIO();

        ImGuiID                mouseViewportID = 0;
        const ImVec2           mousePrevPos    = io.MousePos;
        for (int n = 0; n < platformIO.Viewports.Size; n++)
        {
            ImGuiViewport* viewport = platformIO.Viewports[n];
            auto           data     = GetViewportData(viewport);

            if (data->Window->IsFocused())
            {
                if (io.WantSetMousePos)
                    data->Window->SetCursorPosition(
                        Math::Vec2d((f64)(mousePrevPos.x - viewport->Pos.x),
                                    (f64)(mousePrevPos.y - viewport->Pos.y)));

                if (!bd->MouseWindow)
                {
                    auto cursorPos = data->Window->CursorPosition();
                    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                    {
                        // Single viewport mode: mouse position in client window
                        // coordinates (io.MousePos is (0,0) when the mouse is
                        // on the upper-left corner of the app window)
                        // Multi-viewport mode: mouse position in OS absolute
                        // coordinates (io.MousePos is (0,0) when the mouse is
                        // on the upper-left of the primary monitor)
                        auto windowPos = data->Window->Position();
                        cursorPos += windowPos;
                    }
                    bd->LastValidMousePos
                        = ImVec2((float)cursorPos.x, (float)cursorPos.y);
                    io.AddMousePosEvent((float)cursorPos.x, (float)cursorPos.y);
                }
            }

            if (data->Window->IsHovered()) mouseViewportID = viewport->ID;
        }

        if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
            io.AddMouseViewportEvent(mouseViewportID);
    }
    void ImGuiLayer::UpdateMouseCursor()
    {
        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)) return;

        ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
        ImGuiPlatformIO& platformIO  = ImGui::GetPlatformIO();
        for (i32 n = 0; n < platformIO.Viewports.Size; n++)
        {
            auto window = reinterpret_cast<Window*>(
                platformIO.Viewports[n]->PlatformHandle);

            if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
                window->HideCursor();
            // else
            //  TODO(v1tr10l7): Update cursor
        }
    }
    void ImGuiLayer::UpdateMonitors()
    {
        ImGui_ImplVortex_Data* bd         = GetBackendData();
        ImGuiPlatformIO&       platformIO = ImGui::GetPlatformIO();
        bd->WantUpdateMonitors            = false;
        platformIO.Monitors.resize(0);

        auto toImVec = [](auto x, auto y)
        { return ImVec2(static_cast<f32>(x), static_cast<f32>(y)); };

        if (!Window::HasGlobalPositions())
        {
            // No desktop coordinates: one monitor covering the main window,
            // which keeps menus and tooltips inside it. Floating windows can
            // still go anywhere.
            auto                 size = bd->MainWindow->Size();
            ImGuiPlatformMonitor monitor{};
            monitor.MainPos = monitor.WorkPos = toImVec(0, 0);
            monitor.MainSize = monitor.WorkSize = toImVec(size.x, size.y);
            platformIO.Monitors.push_back(monitor);
            return;
        }

        for (const auto& source : Monitor::Monitors())
        {
            if (!source) continue;
            ImGuiPlatformMonitor monitor{};
            auto                 position = source->Position();
            const auto&          mode     = source->CurrentVideoMode();
            monitor.MainPos  = toImVec(position.x, position.y);
            monitor.MainSize = toImVec(mode.Width, mode.Height);
            auto work        = source->WorkArea();
            if (work.Width > 0 && work.Height > 0)
            {
                monitor.WorkPos  = toImVec(work.X, work.Y);
                monitor.WorkSize = toImVec(work.Width, work.Height);
            }
            else
            {
                monitor.WorkPos  = monitor.MainPos;
                monitor.WorkSize = monitor.MainSize;
            }
            auto scale = source->ContentScale();
            monitor.DpiScale = scale.x > 0.0f ? scale.x : 1.0f;
            platformIO.Monitors.push_back(monitor);
        }
        if (platformIO.Monitors.empty())
        {
            ImGuiPlatformMonitor monitor{};
            monitor.MainPos = monitor.WorkPos = toImVec(0, 0);
            monitor.MainSize = monitor.WorkSize = toImVec(1920, 1080);
            platformIO.Monitors.push_back(monitor);
        }
    }

    void ImGuiLayer::OnKeyPressed(const KeyPressedEvent& e)
    {
        auto                   window      = e.Window;
        auto                   key         = e.KeyCode;
        auto                   repeatCount = e.RepeatCount;

        ImGui_ImplVortex_Data* bd          = GetBackendData();
        if (repeatCount) return;
        UpdateKeyModifiers(key, true);

        usize keycode = ToUnderlying(key);

        if (keycode < bd->KeyOwnerWindows.Size())
            bd->KeyOwnerWindows[keycode] = window;

        ImGuiIO& io       = ImGui::GetIO();
        ImGuiKey imguiKey = ToImGuiKey(key);
        io.AddKeyEvent(imguiKey, true);
        io.SetKeyEventNativeData(imguiKey, static_cast<i32>(key), 0);

        return;
    }
    void ImGuiLayer::OnKeyReleased(const KeyReleasedEvent& e)
    {
        auto                   window = e.Window;
        auto                   key    = e.KeyCode;

        ImGui_ImplVortex_Data* bd     = GetBackendData();
        UpdateKeyModifiers(key, false);

        usize keycode = ToUnderlying(key);
        if (keycode < bd->KeyOwnerWindows.Size())
            bd->KeyOwnerWindows[keycode] = window;

        ImGuiIO& io       = ImGui::GetIO();
        ImGuiKey imguiKey = ToImGuiKey(key);
        io.AddKeyEvent(imguiKey, false);
        io.SetKeyEventNativeData(imguiKey, static_cast<i32>(key), 0);

        return;
    }
    void ImGuiLayer::OnKeyTyped(const KeyTypedEvent& e)
    {
        auto     c  = e.CodePoint;

        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter(c);

        return;
    }

    void ImGuiLayer::OnMouseButtonPressed(const MouseButtonPressedEvent& e)
    {
        auto     button = e.Button;

        ImGuiIO& io     = ImGui::GetIO();
        if (ToUnderlying(button) < 0
            || ToUnderlying(button) >= ImGuiMouseButton_COUNT)
            return;

        io.AddMouseButtonEvent(ToImGuiMouseButton(button), true);
        return;
    }
    void ImGuiLayer::OnMouseButtonReleased(const MouseButtonReleasedEvent& e)
    {
        auto     button = e.Button;

        ImGuiIO& io     = ImGui::GetIO();
        if (ToUnderlying(button) < 0
            || ToUnderlying(button) >= ImGuiMouseButton_COUNT)
            return;

        io.AddMouseButtonEvent(ToImGuiMouseButton(button), false);
        return;
    }
    void ImGuiLayer::OnMouseScrolled(const MouseScrolledEvent& e)
    {
        auto     xoffset = e.DeltaX;
        auto     yoffset = e.DeltaY;

        ImGuiIO& io      = ImGui::GetIO();
        io.AddMouseWheelEvent(static_cast<f32>(xoffset),
                              static_cast<f32>(yoffset));

        return;
    }
    void ImGuiLayer::OnMouseMoved(const MouseMovedEvent& e)
    {
        auto                   window  = e.Window;
        auto                   xoffset = e.X;
        auto                   yoffset = e.Y;

        ImGui_ImplVortex_Data* bd      = GetBackendData();

        ImGuiIO&               io      = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            Math::Vec2i pos = window->Position();
            xoffset += pos.x;
            yoffset += pos.y;
        }

        f32 x = static_cast<f32>(xoffset);
        f32 y = static_cast<f32>(yoffset);

        io.AddMousePosEvent(x, y);
        bd->LastValidMousePos = ImVec2(x, y);
    }
    void ImGuiLayer::OnMouseEntered(const MouseEnteredEvent& e)
    {
        auto                   window = e.Window;
        ImGui_ImplVortex_Data* bd     = GetBackendData();

        ImGuiIO&               io     = ImGui::GetIO();
        bd->MouseWindow               = window;
        io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
    }
    void ImGuiLayer::OnMouseLeft(const MouseLeftEvent& e)
    {
        auto                   window = e.Window;
        ImGui_ImplVortex_Data* bd     = GetBackendData();

        if (bd->MouseWindow == window)
        {
            ImGuiIO& io           = ImGui::GetIO();

            bd->LastValidMousePos = io.MousePos;
            bd->MouseWindow       = nullptr;
            io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
    }

    void ImGuiLayer::OnWindowMoved(const WindowMovedEvent& e)
    {
        auto window = e.Window;

        if (ImGuiViewport* viewport
            = ImGui::FindViewportByPlatformHandle(window))
        {
            if (ImGuiViewportData* vd
                = (ImGuiViewportData*)viewport->PlatformUserData)
            {
                bool ignoreEvent = (ImGui::GetFrameCount()
                                    <= vd->IgnoreWindowPosEventFrame + 1);
                if (ignoreEvent) return;
            }
            viewport->PlatformRequestMove = true;
        }
        return;
    }
    void ImGuiLayer::OnWindowResized(const WindowResizedEvent& event)
    {
        auto window = event.Window;
        // The virtual monitor (Wayland) is the main window's rectangle.
        if (!Window::HasGlobalPositions()) GetBackendData()->WantUpdateMonitors = true;

        if (ImGuiViewport* viewport
            = ImGui::FindViewportByPlatformHandle(window))
        {
            if (ImGuiViewportData* vd
                = (ImGuiViewportData*)viewport->PlatformUserData)
            {
                bool ignoreEvent = (ImGui::GetFrameCount()
                                    <= vd->IgnoreWindowSizeEventFrame + 1);
                if (ignoreEvent) return;
            }
            viewport->PlatformRequestResize = true;
        }
        return;
    }

    void ImGuiLayer::OnWindowFocusGained(const WindowFocusedEvent&)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddFocusEvent(true);
    }
    void ImGuiLayer::OnWindowFocusLost(const WindowFocusLostEvent&)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddFocusEvent(false);
    }
    void ImGuiLayer::OnWindowClosed(const WindowClosedEvent& e)
    {
        auto window = e.Window;

        if (ImGuiViewport* viewport
            = ImGui::FindViewportByPlatformHandle(window))
            viewport->PlatformRequestClose = true;
        return;
    }

    void ImGuiLayer::OnMonitorStateUpdate(const MonitorStateChangedEvent&)
    {
        auto bd                = GetBackendData();
        bd->WantUpdateMonitors = true;

        return;
    }

    void ImGuiLayer::CreateWindow(ImGuiViewport* viewport)
    {
        ImGuiViewportData* data    = new ImGuiViewportData;
        viewport->PlatformUserData = data;

        WindowSpecification specs{};
        specs.Visible     = false;
        specs.Focused     = false;
        specs.FocusOnShow = false;
        specs.Decorated = !(viewport->Flags & ImGuiViewportFlags_NoDecoration);
        specs.AlwaysOnTop      = (viewport->Flags & ImGuiViewportFlags_TopMost);
        specs.VideoMode.Width  = static_cast<i32>(viewport->Size.x);
        specs.VideoMode.Height = static_cast<i32>(viewport->Size.y);
        specs.Title            = "No Title Yet";
        specs.Position         = {static_cast<i32>(viewport->Pos.x),
                                  static_cast<i32>(viewport->Pos.y)};
        bool globalPositions   = Window::HasGlobalPositions();
        if (!globalPositions)
        {
            // Wayland: placed relative to the main window. All popups hang
            // off the main window (not the parent viewport): xdg forbids
            // destroying a popup before its children, and ImGui destroys
            // viewports in any order.
            specs.Kind   = WindowKind::ePopup;
            specs.Parent = &GetBackendData()->MainWindow->Window();
        }
        Window* window           = Window::CreateForImGui(specs);

        data->Window             = window;
        data->WindowOwned        = true;
        viewport->PlatformHandle = (void*)data->Window;
        if (globalPositions)
            data->Window->SetPosition(specs.Position);
    }
    void ImGuiLayer::DestroyWindow(ImGuiViewport* viewport)
    {
        ImGui_ImplVortex_Data* bd = GetBackendData();
        if (auto data = GetViewportData(viewport))
        {
            if (data->WindowOwned && data->Window != &bd->MainWindow->Window())
            {
                // Release any keys that were pressed in the window being
                // destroyed and are still held down, because we will not
                // receive any release events after window is destroyed.
                for (usize i = 0; i < bd->KeyOwnerWindows.Size(); i++)
                    if (bd->KeyOwnerWindows[i] == data->Window)
                        OnKeyReleased(
                            {data->Window, static_cast<Input::KeyCode>(i)});

                delete data->Window;
            }
            delete data;
        }
        viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
    }
    void ImGuiLayer::ShowWindow(ImGuiViewport* viewport)
    {
        GetViewportData(viewport)->Window->SetVisible(true);
    }
    ImVec2 ImGuiLayer::WindowPosition(ImGuiViewport* viewport)
    {
        auto position = GetViewportData(viewport)->Window->Position();

        return ImVec2(static_cast<f32>(position.x),
                      static_cast<f32>(position.y));
    }
    void ImGuiLayer::SetWindowPosition(ImGuiViewport* viewport, ImVec2 position)
    {
        auto data                       = GetViewportData(viewport);
        data->IgnoreWindowPosEventFrame = ImGui::GetFrameCount();

        data->Window->SetPosition(
            {static_cast<i32>(position.x), static_cast<i32>(position.y)});
    }
    ImVec2 ImGuiLayer::WindowSize(ImGuiViewport* viewport)
    {
        auto size = GetViewportData(viewport)->Window->Size();

        return ImVec2(static_cast<f32>(size.x), static_cast<f32>(size.y));
    }
    void ImGuiLayer::SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
    {
        auto data                        = GetViewportData(viewport);
        data->IgnoreWindowSizeEventFrame = ImGui::GetFrameCount();

        data->Window->SetSize(
            Math::Vec2i(static_cast<i32>(size.x), static_cast<i32>(size.y)));
    }
    void ImGuiLayer::SetWindowTitle(ImGuiViewport* viewport, const char* title)
    {
        GetViewportData(viewport)->Window->SetTitle(title);
    }
    void ImGuiLayer::RequestWindowFocus(ImGuiViewport* viewport)
    {
        GetViewportData(viewport)->Window->RequestFocus();
    }
    bool ImGuiLayer::IsWindowFocused(ImGuiViewport* viewport)
    {
        return GetViewportData(viewport)->Window->IsFocused();
    }
    bool ImGuiLayer::IsWindowMinimized(ImGuiViewport* viewport)
    {
        return GetViewportData(viewport)->Window->IsMinimized();
    }
    void ImGuiLayer::SetWindowOpacity(ImGuiViewport* viewport, f32 opacity)
    {
        GetViewportData(viewport)->Window->SetOpacity(opacity);
    }
}; // namespace Vortex
