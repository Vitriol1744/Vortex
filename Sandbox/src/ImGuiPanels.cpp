/*
 * Created by v1tr10l7 on 03.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "ImGuiPanels.hpp"

#include "Vortex/Engine/Application.hpp"

using namespace Vortex;

namespace ImGuiPanels
{
    void DrawWindowOptions(Window& window)
    {
        ImGui::Begin("Window Options");
        static f32 alpha = 0.9f;
        ImGui::SliderFloat("opacity", &alpha, 0.1f, 1.0f);
        window.SetOpacity(alpha);

        if (ImGui::Button("Close")) Application::Get()->Close();
        if (ImGui::Button("Restart")) Application::Get()->Restart();

        static bool fullscreen = false;
        static bool vsync      = false;
        ImGui::Checkbox("Fullscreen", &fullscreen);
        window.SetFullscreen(fullscreen);
        ImGui::Checkbox("VSync", &vsync);
        window.GetRendererContext()->SetVSync(vsync);
        ImGui::End();
    }
} // namespace ImGuiPanels
