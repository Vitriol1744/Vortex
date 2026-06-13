/*
 * Created by v1tr10l7 on 26.05.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Panels/Panel.hpp>
#include <Prism/Utility/Path.hpp>
#include <Vortex/Renderer/API/Texture2D.hpp>

#include <filesystem>

namespace Vortex
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
            OnImGuiRender();
            ImGui::End();
        }

        inline StringView Name() const { return m_Name; }
        bool              Enabled = true;

      protected:
        virtual void OnImGuiRender() override;

      private:
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;

        ::Ref<Texture2D>      m_DirectoryIcon;
        ::Ref<Texture2D>      m_FileIcon;
    };
}; // namespace Vortex
