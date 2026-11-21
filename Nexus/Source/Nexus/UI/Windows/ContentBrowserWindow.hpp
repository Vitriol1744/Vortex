/*
 * Created by v1tr10l7 on 26.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorContext.hpp>

#include <Vortex/Renderer/TextureAsset.hpp>

#include <filesystem>

namespace Vortex::Editor
{
    /// The project's mounted directories and their files.
    class ContentBrowserWindow final : public EditorWindow
    {
      public:
        ContentBrowserWindow();

        void OnDraw(EditorContext& context) override;
        /// Browses the new project's directory.
        void OnProjectChanged(EditorContext&  context,
                              EditorProject& project) override;

      private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_ActiveRootPath;

        ::Ref<TextureAsset>   m_DirectoryIcon;
        ::Ref<TextureAsset>   m_FileIcon;
        ::Ref<TextureAsset>   m_CsIcon;

        void RenderDirectoryTreeNode(const std::filesystem::path& physicalPath,
                                     const std::filesystem::path& rootPath);
    };
}; // namespace Vortex::Editor
