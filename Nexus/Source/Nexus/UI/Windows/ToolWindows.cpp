/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/UI/Windows/ToolWindows.hpp>

#include <Vortex/Renderer/RenderTargetWindow.hpp>

#include <imgui.h>

import Vortex.Renderer;

namespace Vortex::Editor
{
    void StatisticsWindow::OnDraw(EditorContext& context)
    {
        auto& app = context.App;
        ImGui::Text("FPS: %lu", app.FPSCounter());
        ImGui::Text("Frame time: %.3f ms", app.DeltaTime() * 1000.0);

        auto memory = Renderer::MemoryStats();
        ImGui::Text("VRAM: %lu MB / %lu MB", memory.DeviceLocalUsage / 1024ul / 1024ul,
                    memory.DeviceLocalBudget / 1024ul / 1024ul);

        if (auto swapChain = app.Window().SwapChain())
        {
            bool vsync = swapChain->VSync();
            if (ImGui::Checkbox("VSync", &vsync)) swapChain->SetVSync(vsync);
        }

        ImGui::SeparatorText("Viewport camera");
        auto& settings = context.Viewport;
        ImGui::DragFloat("Field of view", &settings.FieldOfView, 0.1f, 20.0f, 150.0f,
                         "%.1f deg");
        f32 planes[2] = {settings.Near, settings.Far};
        if (ImGui::DragFloat2("Near / far", planes, 0.05f, 0.001f, 10000.0f)
            && planes[0] > 0.0f && planes[1] > planes[0])
        {
            settings.Near = planes[0];
            settings.Far  = planes[1];
        }
    }

    void StyleEditorWindow::OnDraw(EditorContext&) { ImGui::ShowStyleEditor(); }

    void ImGuiDemoWindow::OnDraw(EditorContext&) { ImGui::ShowDemoWindow(&Open); }
} // namespace Vortex::Editor
