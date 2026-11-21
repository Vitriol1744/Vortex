#include <Nexus/UI/Modals/NewProjectModal.hpp>
#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Window/Window.hpp>

namespace Vortex::Editor
{
    NewProjectModal::NewProjectModal(OnCreateCallback onCreateCallback)
        : m_OnCreateCallback(Move(onCreateCallback))
    {
        // Provide sane defaults for template selection
        m_Config.Metadata.Name  = "New Project";
        m_Config.StartScenePath = "Assets/Scenes/Main.vscene";

        m_Templates             = ListProjectTemplates();
        for (usize i = 0; i < m_Templates.Size(); ++i)
            if (m_Templates[i].Id == kDefaultTemplate) m_SelectedTemplateIndex = i;
    }

    Path NewProjectModal::TargetPath() const
    {
        return m_ProjectPath / Path(m_Config.Metadata.Name);
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
        if (m_ProjectPath.Empty()) ImGui::TextDisabled("Full Target Path: <Select Location>");
        else ImGui::TextDisabled("Full Target Path: %s", TargetPath().Raw());
    }

    void NewProjectModal::DrawTemplateSelector()
    {
        ImGui::TextUnformatted("Project Template");

        if (m_Templates.Empty())
        {
            ImGui::TextDisabled("No templates in %s; the project starts with the default scene.",
                                kTemplateDirectory);
            return;
        }

        // One selectable card per template: name and category, then the
        // description underneath.
        const f32 cardHeight = ImGui::GetTextLineHeightWithSpacing() * 2.0f + 12.0f;
        for (usize i = 0; i < m_Templates.Size(); ++i)
        {
            const auto& entry    = m_Templates[i];
            const bool  selected = m_SelectedTemplateIndex == i;
            ImGui::PushID(static_cast<int>(i));
            ImVec2 start = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##template", selected, ImGuiSelectableFlags_AllowOverlap,
                                  {0.0f, cardHeight}))
                m_SelectedTemplateIndex = i;
            ImVec2 end = ImGui::GetCursorScreenPos();

            ImGui::SetCursorScreenPos({start.x + 10.0f, start.y + 6.0f});
            ImGui::TextUnformatted(entry.Name.Raw());
            if (!entry.Category.Empty())
            {
                ImGui::SameLine();
                ImGui::TextDisabled("%s", entry.Category.Raw());
            }
            ImGui::SetCursorScreenPos(
                {start.x + 10.0f, start.y + 6.0f + ImGui::GetTextLineHeightWithSpacing()});
            ImGui::TextDisabled("%s", entry.Description.Raw());
            ImGui::SetCursorScreenPos(end);
            ImGui::PopID();
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
        else if (Path target = TargetPath();
                 Filesystem::Exists(target) && !Filesystem::IsEmpty(target))
            m_ValidationError = "That folder already exists and isn't empty.";

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
            {
                StringView templateId = m_Templates.Empty()
                                          ? StringView()
                                          : m_Templates[m_SelectedTemplateIndex].Id.View();
                m_OnCreateCallback(m_Config, TargetPath(), templateId,
                                   m_GenerateGitIgnore);
            }
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
