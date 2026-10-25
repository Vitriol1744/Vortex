/*
 * Created by v1tr10l7 on 17.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/UI/Panel.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Renderer/RHI/Framebuffer.hpp>

namespace Vortex::Editor
{
    class ViewportPanel : public Panel
    {
      public:
        ViewportPanel(StringView name, ::Ref<Framebuffer> targetFramebuffer,
                      bool isEditorCamera)
            : m_Name(name)
            , m_Framebuffer(targetFramebuffer)
            , m_IsEditorCamera(isEditorCamera)
        {
        }

        virtual void OnImGuiRender(bool& isOpen) override
        {
            if (!isOpen) return;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
            ImGui::Begin(m_Name.Raw());

            // 1. Check if the viewport resized in ImGui
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            if (m_ViewportSize.x != viewportPanelSize.x
                || m_ViewportSize.y != viewportPanelSize.y)
            {
                m_ViewportSize = viewportPanelSize;
                m_Framebuffer->OnResize(static_cast<u32>(m_ViewportSize.x),
                                        static_cast<u32>(m_ViewportSize.y));
            }

            // 2. Render the offscreen texture
            auto textureID = UI::TextureFromFramebuffer(m_Framebuffer);
            ImGui::Image(textureID, ImVec2{m_ViewportSize.x, m_ViewportSize.y},
                         ImVec2{0, 1}, ImVec2{1, 0});

            // 3. Handle Viewport Focus & Input Routing
            m_IsFocused = ImGui::IsWindowFocused();
            m_IsHovered = ImGui::IsWindowHovered();

            ImGui::End();
            ImGui::PopStyleVar();
        }

        inline constexpr bool Focused() const { return m_IsFocused; }
        inline constexpr bool Hovered() const { return m_IsHovered; }

      private:
        String             m_Name;
        ::Ref<Framebuffer> m_Framebuffer;
        ImVec2             m_ViewportSize{0, 0};
        bool               m_IsEditorCamera = false;
        bool               m_IsFocused      = false;
        bool               m_IsHovered      = false;
    };
}; // namespace Vortex::Editor
