#include <Nexus/UI/Modals/NewProjectModal.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex::Editor
{
    NewProjectModal::NewProjectModal(OnCreateCallback onCreateCallback)
        : m_OnCreateCallback(Move(onCreateCallback))
    {
        // Provide sane defaults for template selection
        m_Config.Metadata.Name  = "New Project";
        m_Config.StartScenePath = "Assets/Scenes/Main.vscene";
    }

    bool NewProjectModal::OnImGuiRender()
    {
        bool keepModalOpen = true;

        DrawHeader();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        DrawFormFields();
        ImGui::Spacing();

        DrawTemplateSelector();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (DrawFooterButtons()) keepModalOpen = false;

        return keepModalOpen;
    }

    void NewProjectModal::DrawHeader()
    {
        ImGui::PushStyleColor(ImGuiCol_Text,
                              ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
        ImGui::TextUnformatted("VORTEX ENGINE");
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::TextDisabled("| Project Initialization Wizard");
    }

    void NewProjectModal::DrawFormFields()
    {
        if (ImGui::BeginTable("NewProjectFormTable", 2,
                              ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed,
                                    120.0f);
            ImGui::TableSetupColumn("Input",
                                    ImGuiTableColumnFlags_WidthStretch);

            // --- Project Name ---
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Project Name");

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1.0f);
            UI::InputText("##project_name", m_Config.Metadata.Name);

            // --- Location Path + Browse ---
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Location");

            ImGui::TableSetColumnIndex(1);
            float browseButtonWidth = 80.0f;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x
                                    - browseButtonWidth
                                    - ImGui::GetStyle().ItemSpacing.x);
            UI::InputText("##project_location", m_ProjectPath);

            ImGui::SameLine();
            if (ImGui::Button("Browse...", ImVec2(browseButtonWidth, 0.0f)))
            {
                if (auto maybePath = Window::OpenFolder("."))
                    m_ProjectPath = *maybePath;
            }

            // --- Version Control Option ---
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Git");

            ImGui::TableSetColumnIndex(1);
            ImGui::Checkbox("Initialize .gitignore", &m_GenerateGitIgnore);

            ImGui::EndTable();
        }

        // Preview full creation path
        String fullPath
            = m_ProjectPath.Empty() ? "<Select Location>" : m_ProjectPath.Raw();
        ImGui::TextDisabled("Full Target Path: %s/%s", fullPath.Raw(),
                            m_Config.Metadata.Name.Raw());
    }

    void NewProjectModal::DrawTemplateSelector()
    {
        ImGui::TextUnformatted("Project Template");

        if (m_Templates.Empty()) return;

        if (ImGui::BeginCombo("##template_combo",
                              m_Templates[m_SelectedTemplateIndex].Raw()))
        {
            for (usize i = 0; i < m_Templates.Size(); ++i)
            {
                const bool isSelected = (m_SelectedTemplateIndex == i);
                if (ImGui::Selectable(m_Templates[i].Raw(), isSelected))
                    m_SelectedTemplateIndex = i;
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    bool NewProjectModal::DrawFooterButtons()
    {
        bool closeRequested = false;

        // Validation Logic
        m_ValidationError.Clear();
        if (m_Config.Metadata.Name.Empty())
            m_ValidationError = "Project name cannot be empty.";
        else if (m_ProjectPath.Empty())
            m_ValidationError = "Project location path cannot be empty.";

        if (!m_ValidationError.Empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 0.36f, 0.36f, 1.0f));
            ImGui::TextUnformatted(m_ValidationError.Raw());
            ImGui::PopStyleColor();
        }
        else ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeight()));

        // Button Alignment
        constexpr f32 buttonWidth = 120.0f;
        const f32     rightPadding
            = (buttonWidth * 2.0f) + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - rightPadding
                             - ImGui::GetStyle().WindowPadding.x);

        const bool canCreate = m_ValidationError.Empty();
        if (!canCreate) ImGui::BeginDisabled();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.55f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.3f, 0.68f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.15f, 0.45f, 0.15f, 1.0f));

        if (ImGui::Button("Create Project", ImVec2(buttonWidth, 0.0f)))
        {
            if (m_OnCreateCallback)
                // Pass both config and target path back to listener!
                m_OnCreateCallback(m_Config, m_ProjectPath,
                                   m_GenerateGitIgnore);
            closeRequested = true;
        }

        ImGui::PopStyleColor(3);

        if (!canCreate) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)))
            closeRequested = true;

        return closeRequested;
    }
} // namespace Vortex::Editor
