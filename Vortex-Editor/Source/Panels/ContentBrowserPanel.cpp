/*
 * Created by v1tr10l7 on 26.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Panels/ContentBrowserPanel.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp>
#include <Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h>
#include <Vortex/Renderer/ImGui/ImGui.hpp>

namespace Vortex
{
    ContentBrowserPanel::ContentBrowserPanel()
        : Panel("Content Browser")
        , m_BaseDirectory("assets")
        , m_CurrentDirectory(m_BaseDirectory)
    {
        m_DirectoryIcon = Texture2D::Create(
            "assets/icons/content-browser/directory-icon.png");
        m_FileIcon
            = Texture2D::Create("assets/icons/content-browser/file-icon.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        static f32 padding       = 16.0f;
        static f32 thumbnailSize = 50.0f;
        f32        cellSize      = thumbnailSize + padding;

        f32        panelWidth    = ImGui::GetContentRegionAvail().x;
        i32        columnCount   = (i32)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        // --- FIXED: Render centered text ".." instead of an icon ---
        if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
        {
            ImGui::PushID(".._back_dir");

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                ImVec4(0.3f, 0.3f, 0.3f, 0.35f)); // Light highlight on hover
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

            ImVec2 startCursorPos = ImGui::GetCursorPos();
            ImGui::Button("##back_btn", {thumbnailSize, thumbnailSize});

            if (ImGui::IsItemHovered()
                && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            ImGui::PopStyleColor(3);

            String backText = "..";
            ImVec2 textSize = ImGui::CalcTextSize(backText.Raw());

            // Set cursor position to the center of the button we just drew
            float  offsetX  = (thumbnailSize - textSize.x) * 0.5f;
            float  offsetY  = (thumbnailSize - textSize.y) * 0.5f;
            ImGui::SetCursorPos(
                ImVec2(startCursorPos.x + offsetX, startCursorPos.y + offsetY));

            // Render the centered text
            ImGui::TextUnformatted(backText.Raw());
            ImGui::SetCursorPos(ImVec2(startCursorPos.x,
                                       startCursorPos.y + thumbnailSize
                                           + ImGui::GetStyle().ItemSpacing.y));
            ImGui::NextColumn();
            ImGui::PopID();
        }
        // -----------------------------------------------------------        //
        // ---------------------------------------------------------

        // Your existing directory iterator continues seamlessly below
        for (auto& directoryEntry :
             std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path           = directoryEntry.path();
            std::string filenameString = path.filename().string();

            ImGui::PushID(filenameString.c_str());
            ::Ref<Texture2D> icon
                = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton((ImTextureID)UI::GetTexture(icon),
                               {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

            if (ImGui::BeginDragDropSource())
            {
                // FIX: Extract string out of path cleanly
                std::string pathString   = path.string();
                const char* itemPathCStr = pathString.c_str();

                // Pass the actual string buffer; ImGui copies this memory
                // safely internally
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPathCStr,
                                          pathString.size() + 1);

                ImGui::TextUnformatted(filenameString.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()
                && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
            }
            ImGui::TextWrapped("%s", filenameString.c_str());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::End();
    }
}; // namespace Vortex
