/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Project/Project.hpp>
#include <Vortex/Renderer/ImGui/ModalManager.hpp>

namespace Vortex::Editor
{
    class NewProjectModal : public UI::Modal
    {
      public:
        using OnCreateCallback
            = Delegate<void(const ProjectConfig& config, PathView path,
                            bool generateGitIgnore)>;

        explicit NewProjectModal(OnCreateCallback onCreateCallback);

        const char* Title() const override
        {
            return "Create New Project##NewProjectModal";
        }
        bool   OnImGuiRender() override;
        ImVec2 MinSize() const override { return {680.0f, 0.0f}; }

      private:
        void           DrawHeader();
        void           DrawFormFields();
        void           DrawTemplateSelector();
        void           DrawFooter();
        bool           DrawFooterButtons();

        ProjectConfig  m_Config;
        Path           m_ProjectPath;
        bool           m_GenerateGitIgnore     = true;

        int            m_SelectedTemplateIndex = 0;
        Vector<String> m_Templates
            = {"Blank (2D/3D)", "2D Physics Starter", "3D Lit Sandbox"};

        OnCreateCallback m_OnCreateCallback;
        String           m_ValidationError;
    };
}; // namespace Vortex::Editor
