/*
 * Created by v1tr10l7 on 14.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Project/Project.hpp>
#include <Vortex/Renderer/ImGui/ModalManager.hpp>

import Nexus.ProjectTemplates;

namespace Vortex::Editor
{
    class NewProjectModal : public UI::Modal
    {
      public:
        /// `path` is the new project's own directory (location/name).
        using OnCreateCallback
            = Delegate<void(const ProjectConfig& config, PathView path,
                            StringView templateId, bool generateGitIgnore)>;

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

        /// The engine's templates (assets/templates).
        Vector<ProjectTemplate> m_Templates;
        usize                   m_SelectedTemplateIndex = 0;

        Path                    TargetPath() const;

        OnCreateCallback m_OnCreateCallback;
        String           m_ValidationError;
    };
}; // namespace Vortex::Editor
