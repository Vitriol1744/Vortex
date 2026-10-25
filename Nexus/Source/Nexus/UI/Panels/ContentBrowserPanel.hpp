/*
 * Created by v1tr10l7 on 26.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/UI/Panel.hpp>
#include <Prism/Utility/Path.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>

#include <filesystem>

namespace Vortex::Editor
{
    class ContentBrowserPanel final : public Panel
    {
      public:
        ContentBrowserPanel();
        explicit ContentBrowserPanel(StringView name)
            : Panel(name)
        {
        }

        inline void Draw()
        {
            if (!Enabled) return;
            ImGui::Begin(m_Name.Raw(), &Enabled);
            OnImGuiRender(Enabled);
            ImGui::End();
        }

        inline StringView Name() const { return m_Name; }
        bool              Enabled = true;

      protected:
        virtual void OnImGuiRender(bool& isOpen) override;

      private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_ActiveRootPath;

        ::Ref<TextureAsset>   m_DirectoryIcon;
        ::Ref<TextureAsset>   m_FileIcon;
        ::Ref<TextureAsset>   m_CsIcon;

        void RenderDirectoryTreeNode(const std::filesystem::path& physicalPath,
                                     const std::filesystem::path& rootPath);
        void OnProjectChanged(const ProjectChangedEvent& e) override;
    };
}; // namespace Vortex::Editor
