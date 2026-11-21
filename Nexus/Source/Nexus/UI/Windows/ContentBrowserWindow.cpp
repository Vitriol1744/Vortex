/*
 * Created by v1tr10l7 on 26.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorProject.hpp>
#include <Nexus/EditorResources.hpp>
#include <Nexus/UI/Windows/ContentBrowserWindow.hpp>

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>

namespace Vortex::Editor
{
    ContentBrowserWindow::ContentBrowserWindow()
        : EditorWindow("Project")
        , m_CurrentDirectory("assets")
        , m_ActiveRootPath("assets")
    {
        m_DirectoryIcon = CreateRef<TextureAsset>(
            "assets/icons/content-browser/directory-icon.png");
        m_FileIcon = CreateRef<TextureAsset>(
            "assets/icons/content-browser/file-icon.png");
        m_CsIcon = CreateRef<TextureAsset>(
            "assets/icons/content-browser/cs-file-icon.png");
    }

    void ContentBrowserWindow::OnDraw(EditorContext&)
    {
        ImVec2     availableRegion = ImGui::GetContentRegionAvail();
        static f32 leftTreeWidth   = 220.0f; // Left panel size

        // ====================================================================
        // LEFT PANEL: Virtual Project Tree (Mount Points)
        // ====================================================================
        ImGui::BeginChild("FolderTreeChild",
                          ImVec2(leftTreeWidth, availableRegion.y),
                          ImGuiChildFlags_Borders);

        for (const auto& [virtualPath, physicalPath] :
             Filesystem::MountPoints())
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                                     | ImGuiTreeNodeFlags_OpenOnDoubleClick
                                     | ImGuiTreeNodeFlags_AllowOverlap;

            bool isSelected
                = (PathView(m_ActiveRootPath.string().data()) == physicalPath
                   && PathView(m_CurrentDirectory.string().data())
                          == physicalPath);
            if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

            bool isNodeOpen
                = ImGui::TreeNodeEx(virtualPath.Raw(), flags, "%s", "");

            // Add full-width clickable selection behavior
            ImGui::SameLine();
            if (ImGui::Selectable(("##mount_select_" + virtualPath).Raw(),
                                  isSelected, ImGuiSelectableFlags_AllowOverlap,
                                  ImVec2(0, ImGui::GetTextLineHeight())))
            {
                m_CurrentDirectory = physicalPath.Raw();
                m_ActiveRootPath   = physicalPath.Raw();
            }

            // Draw the icon
            ImGui::SameLine();
            const float treeIconSize = 16.0f;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()
                                 + (ImGui::GetTextLineHeight() - treeIconSize)
                                       * 0.5f);
            ImGui::Image(
                UI::TextureID(m_DirectoryIcon->Texture()),
                ImVec2(treeIconSize, treeIconSize), ImVec2(0, 1), ImVec2(1, 0));

            // Print text
            ImGui::SameLine();
            ImGui::TextUnformatted(virtualPath.Raw());

            if (isNodeOpen)
            {
                for (const auto& entry :
                     std::filesystem::directory_iterator(physicalPath.Raw()))
                {
                    if (entry.is_directory())
                        RenderDirectoryTreeNode(entry.path(),
                                                physicalPath.Raw());
                }
                // RenderDirectoryTreeNode(physicalPath.Raw(),
                // physicalPath.Raw());
                ImGui::TreePop();
            }
        }

        ImGui::EndChild(); // Chain the window sections tightly
        ImGui::SameLine();

        // ====================================================================
        // RIGHT PANEL: Existing Content Grid View
        // ====================================================================
        ImGui::BeginChild("FileGridChild", ImVec2(0, availableRegion.y),
                          ImGuiChildFlags_None);

        static f32 padding       = 16.0f;
        static f32 thumbnailSize = 50.0f;
        f32        cellSize      = thumbnailSize + padding;

        f32        panelWidth    = ImGui::GetContentRegionAvail().x;
        i32        columnCount   = (i32)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        if (m_CurrentDirectory != m_ActiveRootPath)
        {
            ImGui::PushID(".._back_dir");

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.3f, 0.3f, 0.3f, 0.35f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

            ImVec2 startCursorPos = ImGui::GetCursorPos();
            ImGui::Button("##back_btn", {thumbnailSize, thumbnailSize});

            if (ImGui::IsItemHovered()
                && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                m_CurrentDirectory = m_CurrentDirectory.parent_path();

            ImGui::PopStyleColor(3);

            String backText = "..";
            ImVec2 textSize = ImGui::CalcTextSize(backText.Raw());

            float  offsetX  = (thumbnailSize - textSize.x) * 0.5f;
            float  offsetY  = (thumbnailSize - textSize.y) * 0.5f;
            ImGui::SetCursorPos(
                ImVec2(startCursorPos.x + offsetX, startCursorPos.y + offsetY));

            ImGui::TextUnformatted(backText.Raw());

            // Advance cursor position to the bottom of the thumbnail area
            ImGui::SetCursorPos(ImVec2(startCursorPos.x,
                                       startCursorPos.y + thumbnailSize
                                           + ImGui::GetStyle().ItemSpacing.y));

            // FIX: Inform ImGui's layout system of the extended boundary
            ImGui::Dummy(ImVec2(thumbnailSize, 0.0f));

            ImGui::NextColumn();
            ImGui::PopID();
        }
        for (auto& directoryEntry :
             std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path           = directoryEntry.path();
            Path        filenameString = path.filename().c_str();

            ImGui::PushID(filenameString.Raw());
            ::Ref<RHI::Texture> icon = directoryEntry.is_directory()
                                      ? m_DirectoryIcon->Texture()
                                      : m_FileIcon->Texture();
            if (filenameString.Extension() == "cs") icon = m_CsIcon->Texture();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton(filenameString.Raw(),
                               UI::TextureID(icon),
                               {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

            if (ImGui::BeginDragDropSource())
            {
                std::string pathString   = path.string();
                const char* itemPathCStr = pathString.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPathCStr,
                                          pathString.size() + 1);

                ImGui::TextUnformatted(filenameString.Raw());
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()
                && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();
            }
            ImGui::TextWrapped("%s", filenameString.Raw());

            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::EndChild();
    }
    void ContentBrowserWindow::RenderDirectoryTreeNode(
        const std::filesystem::path& physicalPath,
        const std::filesystem::path& rootPath)
    {
        if (!std::filesystem::exists(physicalPath))
        {
            VtCoreWarn(
                "ContentBrowserWindow: mountpoint's physical path does not "
                "exist: `{}`",
                physicalPath.string().data());
            return;
        }

        ImGuiTreeNodeFlags flags
            = ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_OpenOnDoubleClick; // Keyboard nav polish

        bool isSelected = (m_CurrentDirectory == physicalPath);
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

        bool hasSubdirectories = false;
        try
        {
            for (const auto& entry :
                 std::filesystem::directory_iterator(physicalPath))
            {
                if (entry.is_directory())
                {
                    hasSubdirectories = true;
                    break;
                }
            }
        }
        catch (...)
        {
        }

        if (!hasSubdirectories) flags |= ImGuiTreeNodeFlags_Leaf;

        std::string filename = physicalPath.filename().string();
        ImGuiID     nodeID = (ImGuiID)std::filesystem::hash_value(physicalPath);

        // 1. Advance the tree layout structure without pushing content text
        // ImGuiTreeNodeFlags_AllowOverlap allows Selectable to sit on top of
        // this row zone
        bool        isNodeOpen = ImGui::TreeNodeEx(
            (void*)(uintptr_t)nodeID, flags | ImGuiTreeNodeFlags_AllowOverlap,
            "%s", "");

        // 2. Map a Selectable hit box across the remainder of the layout row
        // line
        ImGui::SameLine();

        // Using a Selectable ensures the entire row length handles click inputs
        // cleanly
        if (ImGui::Selectable(("##selectable_" + filename).c_str(), isSelected,
                              ImGuiSelectableFlags_AllowOverlap,
                              ImVec2(0, ImGui::GetTextLineHeight())))
        {
            m_CurrentDirectory = physicalPath;
            m_ActiveRootPath   = rootPath;
            VtCoreTrace("Current directory => {}\nActive Root => {}",
                        m_CurrentDirectory.string().data(),
                        m_ActiveRootPath.string().data());
        }

        // 3. Render your Directory Custom Icon Over the row layer
        ImGui::SameLine();
        const float treeIconSize = 16.0f;
        // Align vertically within the text line height bounds
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()
                             + (ImGui::GetTextLineHeight() - treeIconSize)
                                   * 0.5f);

        ImGui::Image(UI::TextureID(m_DirectoryIcon->Texture()),
                     ImVec2(treeIconSize, treeIconSize), ImVec2(0, 1),
                     ImVec2(1, 0));

        // 4. Draw the actual directory label text
        ImGui::SameLine();
        ImGui::TextUnformatted(filename.c_str());

        // 5. Recurse down children if the node structure state is unfolded open
        if (isNodeOpen)
        {
            if (hasSubdirectories)
            {
                for (const auto& entry :
                     std::filesystem::directory_iterator(physicalPath))
                {
                    if (entry.is_directory())
                        RenderDirectoryTreeNode(entry.path(), rootPath);
                }
            }
            ImGui::TreePop();
        }
    }

    void ContentBrowserWindow::OnProjectChanged(EditorContext&,
                                                EditorProject& project)
    {
        m_CurrentDirectory = project.Path().Raw();
        m_ActiveRootPath   = project.Path().Raw();
    }
}; // namespace Vortex::Editor
