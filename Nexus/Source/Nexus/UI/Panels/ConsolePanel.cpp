/*
 * Created by v1tr10l7 on 25.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/UI/Panels/ConsolePanel.hpp>
#include <Prism/String/StringUtils.hpp>

#include <fstream>
#include <sstream>

namespace Vortex::Editor
{
    extern Scope<ConsoleSink> g_ConsoleSink;

    using Prism::LogLevel;
    ConsolePanel::ConsolePanel()
        : Panel("Console")
    {
        g_ConsoleSink->SetTargetPanel(RefFromThis().As<ConsolePanel>());
        PopulateSampleLogs();
    }

    void ConsolePanel::AddLog(LogEntry entry)
    {
        if (m_CollapseLogs && !m_Logs.Empty())
        {
            auto& last = m_Logs.Back();
            if (last.Level == entry.Level && last.Tag == entry.Tag
                && last.Source == entry.Source && last.Payload == entry.Payload
                && last.SourceFile == entry.SourceFile
                && last.SourceLine == entry.SourceLine)
            {
                last.Count++;
                last.Timestamp = entry.Timestamp;
                return;
            }
        }
        m_Logs.PushBack(entry);
    }

    void ConsolePanel::PopulateSampleLogs()
    {
        m_Logs.Clear();
        AddLog({"12:04:00", LogLevel::eDebug, "Core",
                "Allocated 1024 bytes in temporary arena allocator.",
                "ArenaAllocator.cpp", 15, 1, LogSource::eEngine});
        AddLog({"12:04:01", LogLevel::eInfo, "Engine",
                "Vortex Engine v0.4.2 initialized successfully.", "Engine.cpp",
                42, 1, LogSource::eEngine});
        AddLog({"12:04:01", LogLevel::eInfo, "Render",
                "Initializing Vulkan Swapchain with format B8G8R8A8_UNORM...",
                "VulkanContext.cpp", 128, 1, LogSource::eEngine});
        AddLog({"12:04:02", LogLevel::eWarn, "Physics",
                "High velocity collision detected on Entity #12. Discrete "
                "fallback used.",
                "RigidBody.cpp", 87, 1, LogSource::eEngine});
        AddLog({"12:04:02", LogLevel::eWarn, "Physics",
                "High velocity collision detected on Entity #12. Discrete "
                "fallback used.",
                "RigidBody.cpp", 87, 1, LogSource::eEngine});
        AddLog({"12:04:02", LogLevel::eWarn, "Physics",
                "High velocity collision detected on Entity #12. Discrete "
                "fallback used.",
                "RigidBody.cpp", 87, 1, LogSource::eEngine});
        AddLog({"12:04:03", LogLevel::eError, "Asset",
                "Failed to load texture file: "
                "'Assets/Textures/T_Character_D.png'.",
                "AssetManager.cpp", 112, 1, LogSource::eEngine});
        AddLog({"12:04:04", LogLevel::eInfo, "Scripting",
                "Mono C# Domain reloaded in 14.2ms.", "ScriptEngine.cpp", 204,
                1, LogSource::eEngine});
        AddLog({"12:04:04", LogLevel::eInfo, "Gameplay",
                "Player spawned at position (0.0, 1.5, 0.0)",
                "PlayerController.cs", 34, 1, LogSource::eClient});
        AddLog(
            {"12:04:05", LogLevel::eWarn, "Gameplay",
             "Health pack missing drop probability table. Defaulting to 1.0.",
             "HealthComponent.cs", 82, 1, LogSource::eClient});
        AddLog(
            {"12:04:05", LogLevel::eWarn, "Audio",
             "Sound buffer 'S_Explosion.wav' exceeded max channel allocation.",
             "AudioEngine.cpp", 65, 1, LogSource::eEngine});
        AddLog(
            {"12:04:05", LogLevel::eWarn, "Audio",
             "Sound buffer 'S_Explosion.wav' exceeded max channel allocation.",
             "AudioEngine.cpp", 65, 1, LogSource::eEngine});
        AddLog({"12:04:06", LogLevel::eError, "Graphics",
                "Shader compilation failed: Line 45 syntax error near 'mat4'.",
                "ShaderCompiler.cpp", 45, 1, LogSource::eEngine});
        AddLog({"12:04:07", LogLevel::eFatal, "Memory",
                "Fatal out-of-memory error! Unable to allocate VRAM buffer.",
                "VulkanMemory.cpp", 310, 1, LogSource::eEngine});
    }

    void ConsolePanel::OnImGuiRender(bool& isOpen)
    {
        if (g_ConsoleSink) g_ConsoleSink->Flush();

        RenderToolbar();
        ImGui::Separator();

        float availableHeight = ImGui::GetContentRegionAvail().y;
        float listHeight      = availableHeight - m_DetailPaneHeight
                         - ImGui::GetStyle().ItemSpacing.y;
        if (listHeight < 50.0f) listHeight = 50.0f;

        RenderLogList(listHeight);

        ImGui::Button("##Splitter", ImVec2(-1, 3.0f));
        if (ImGui::IsItemActive())
        {
            m_DetailPaneHeight -= ImGui::GetIO().MouseDelta.y;
            if (m_DetailPaneHeight < 50.0f) m_DetailPaneHeight = 50.0f;
            if (m_DetailPaneHeight > 300.0f) m_DetailPaneHeight = 300.0f;
        }

        RenderDetailInspector();
    }

    void ConsolePanel::UpdateAvailableTags()
    {
        for (usize i = 0; i < m_Logs.Size(); ++i)
        {
            const auto& log    = m_Logs[i];
            String      tagKey = log.Tag.Empty() ? "Untagged" : log.Tag;
            if (m_TagFilters.Find(tagKey) == m_TagFilters.end())
                m_TagFilters[tagKey] = true;
        }
    }

    void ConsolePanel::RenderCategoryFilter()
    {
        UpdateAvailableTags();

        usize activeCount = 0;
        for (const auto& [tag, enabled] : m_TagFilters)
        {
            if (enabled) ++activeCount;
        }

        String previewText;
        if (activeCount == m_TagFilters.Size())
            previewText = "Categories (All)";
        else if (activeCount == 0) previewText = "Categories (None)";
        else
            previewText = "Categories (" + StringUtils::ToString(activeCount)
                        + "/" + StringUtils::ToString(m_TagFilters.Size())
                        + ")";

        ImGui::SetNextItemWidth(160.0f);
        if (ImGui::BeginCombo("##CategoryFilter", previewText.Raw()))
        {
            if (ImGui::Checkbox("Select All", &m_SelectAllTags))
            {
                for (auto& [tag, enabled] : m_TagFilters)
                    enabled = m_SelectAllTags;
            }

            ImGui::Separator();

            for (auto& [tag, enabled] : m_TagFilters)
            {
                String displayLabel  = tag.Empty() ? "Untagged" : tag;
                String checkboxLabel = displayLabel + "##" + tag;

                ImGui::PushID(
                    static_cast<int>(std::hash<std::string>{}(tag.Raw())));
                ImGui::Checkbox(checkboxLabel.Raw(), &enabled);
                ImGui::PopID();
            }

            ImGui::EndCombo();
        }
    }

    void ConsolePanel::RenderSourceFilter()
    {
        usize activeCount = 0;
        if (m_ShowEngine) ++activeCount;
        if (m_ShowClient) ++activeCount;

        String previewText;
        if (activeCount == 2) previewText = "Source (All)";
        else if (activeCount == 0) previewText = "Source (None)";
        else
            previewText
                = "Source (" + StringUtils::ToString(activeCount) + "/2)";

        ImGui::SetNextItemWidth(130.0f);
        if (ImGui::BeginCombo("##SourceFilter", previewText.Raw()))
        {
            if (ImGui::Checkbox("Select All", &m_SelectAllSources))
            {
                m_ShowEngine = m_SelectAllSources;
                m_ShowClient = m_SelectAllSources;
            }

            ImGui::Separator();
            ImGui::Checkbox("Engine", &m_ShowEngine);
            ImGui::Checkbox("Client", &m_ShowClient);

            ImGui::EndCombo();
        }
    }

    void ConsolePanel::RenderLogLevelFilter()
    {
        usize activeCount = 0;
        if (m_ShowDebug) ++activeCount;
        if (m_ShowInfo) ++activeCount;
        if (m_ShowWarn) ++activeCount;
        if (m_ShowError) ++activeCount;
        if (m_ShowFatal) ++activeCount;

        String previewText;
        if (activeCount == 5) previewText = "Levels (All)";
        else if (activeCount == 0) previewText = "Levels (None)";
        else
            previewText
                = "Levels (" + StringUtils::ToString(activeCount) + "/5)";

        ImGui::SetNextItemWidth(140.0f);
        if (ImGui::BeginCombo("##LogLevelFilter", previewText.Raw()))
        {
            if (ImGui::Checkbox("Select All", &m_SelectAllLogLevels))
            {
                m_ShowDebug = m_SelectAllLogLevels;
                m_ShowInfo  = m_SelectAllLogLevels;
                m_ShowWarn  = m_SelectAllLogLevels;
                m_ShowError = m_SelectAllLogLevels;
                m_ShowFatal = m_SelectAllLogLevels;
            }

            ImGui::Separator();
            ImGui::Checkbox("Debug", &m_ShowDebug);
            ImGui::Checkbox("Info", &m_ShowInfo);
            ImGui::Checkbox("Warn", &m_ShowWarn);
            ImGui::Checkbox("Error", &m_ShowError);
            ImGui::Checkbox("Fatal", &m_ShowFatal);

            ImGui::EndCombo();
        }
    }

    void ConsolePanel::RenderToolbar()
    {
        if (ImGui::Button("Clear"))
        {
            m_Logs.Clear();
            m_SelectedIndex = -1;
        }

        ImGui::SameLine();
        if (ImGui::Button("Copy")) CopyToClipboard();

        ImGui::SameLine();
        if (ImGui::Button("Export")) ExportLogFile("console_export.log");

        ImGui::SameLine();
        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputTextWithHint("##Filter", "Search...", m_FilterBuffer,
                                 sizeof(m_FilterBuffer));

        ImGui::SameLine();
        RenderCategoryFilter();

        ImGui::SameLine();
        RenderSourceFilter();

        ImGui::SameLine();
        RenderLogLevelFilter();

        ImGui::SameLine();
        ImGui::Checkbox("Collapse", &m_CollapseLogs);

        ImGui::SameLine();
        ImGui::Checkbox("Auto-Scroll", &m_AutoScroll);
    }

    void ConsolePanel::RenderLogList(float height)
    {
        ImGui::BeginChild("LogListRegion", ImVec2(0, height), true,
                          ImGuiWindowFlags_HorizontalScrollbar);

        String filterStr = m_FilterBuffer;

        for (usize i = 0; i < m_Logs.Size(); ++i)
        {
            const auto& entry = m_Logs[i];

            if (entry.Source == LogSource::eEngine && !m_ShowEngine) continue;
            if (entry.Source == LogSource::eClient && !m_ShowClient) continue;

            // Apply Level Filtering
            if (entry.Level == LogLevel::eDebug && !m_ShowDebug) continue;
            if (entry.Level == LogLevel::eInfo && !m_ShowInfo) continue;
            if (entry.Level == LogLevel::eWarn && !m_ShowWarn) continue;
            if (entry.Level == LogLevel::eError && !m_ShowError) continue;
            if (entry.Level == LogLevel::eFatal && !m_ShowFatal) continue;

            // Apply Tag Filtering
            auto tagIt = m_TagFilters.Find(entry.Tag);
            if (tagIt != m_TagFilters.end() && !tagIt->Value) continue;

            // Apply Text Filtering
            if (!filterStr.Empty())
            {
                if (entry.Payload.Find(filterStr) == String::NPos
                    && entry.Tag.Find(filterStr) == String::NPos)
                    continue;
            }

            ImGui::PushID(static_cast<int>(i));

            // Color coding based on LogLevel
            ImVec4 textColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
            if (entry.Level == LogLevel::eDebug)
                textColor = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
            else if (entry.Level == LogLevel::eWarn)
                textColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
            else if (entry.Level == LogLevel::eError)
                textColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            else if (entry.Level == LogLevel::eFatal)
                textColor = ImVec4(1.0f, 0.1f, 0.4f, 1.0f);

            String sourceStr
                = (entry.Source == LogSource::eEngine) ? "Engine" : "Client";
            String label = "[" + entry.Timestamp + "] [" + sourceStr + "] ["
                         + entry.Tag + "] " + entry.Payload;

            ImGui::PushStyleColor(ImGuiCol_Text, textColor);

            // If the item has multiple occurrences, render badge inline
            if (m_CollapseLogs && entry.Count > 1)
            {
                String badgeStr = "x" + StringUtils::ToString(entry.Count);

                // Draw selectable item
                if (ImGui::Selectable(label.Raw(),
                                      m_SelectedIndex == static_cast<i32>(i)))
                    m_SelectedIndex = static_cast<i32>(i);

                // Right-align/append badge pill on same line
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button,
                                      ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
                ImGui::SmallButton(badgeStr.Raw());
                ImGui::PopStyleColor(2);
            }
            else {
                if (ImGui::Selectable(label.Raw(),
                                      m_SelectedIndex == static_cast<i32>(i)))
                    m_SelectedIndex = static_cast<i32>(i);
            }

            ImGui::PopStyleColor();

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Copy Log Line"))
                {
                    String fullLine = label;
                    if (entry.Count > 1)
                        fullLine
                            += " (x" + StringUtils::ToString(entry.Count) + ")";
                    ImGui::SetClipboardText(fullLine.Raw());
                }
                if (ImGui::MenuItem("Copy Payload"))
                    ImGui::SetClipboardText(entry.Payload.Raw());
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }

    void ConsolePanel::RenderDetailInspector()
    {
        ImGui::BeginChild("LogDetailRegion", ImVec2(0, 0), true);

        if (m_SelectedIndex >= 0
            && m_SelectedIndex < static_cast<i32>(m_Logs.Size()))
        {
            const auto& entry = m_Logs[m_SelectedIndex];

            ImGui::TextUnformatted(entry.Payload.Raw());

            if (entry.Count > 1)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(Repeated %u times)", entry.Count);
            }

            if (!entry.SourceFile.Empty())
            {
                ImGui::Separator();
                ImGui::TextDisabled("Location:");
                ImGui::SameLine();

                String linkText = entry.SourceFile + ":"
                                + StringUtils::ToString(entry.SourceLine);

                ImGui::PushStyleColor(ImGuiCol_Text,
                                      ImVec4(0.35f, 0.68f, 1.0f, 1.0f));
                if (ImGui::Button(linkText.Raw()))
                    OpenFileInIDE(m_PreferredIDE, entry.SourceFile,
                                  entry.SourceLine);
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Click to open in %s",
                                      m_PreferredIDE == IDEType::eVSCode
                                          ? "VS Code"
                                          : "Visual Studio");
            }
        }
        else
            ImGui::TextDisabled(
                "Select a log entry to view full stack trace and source "
                "location.");

        ImGui::EndChild();
    }

    void ConsolePanel::CopyToClipboard()
    {
        std::ostringstream ss;
        String             filterStr = m_FilterBuffer;

        if (m_SelectedIndex >= 0
            && m_SelectedIndex < static_cast<i32>(m_Logs.Size()))
        {
            const auto& entry = m_Logs[m_SelectedIndex];
            String      sourceStr
                = (entry.Source == LogSource::eEngine) ? "Engine" : "Client";
            ss << "[" << entry.Timestamp.Raw() << "] [" << sourceStr.Raw()
               << "] [" << entry.Tag.Raw() << "] " << entry.Payload.Raw();
            if (m_CollapseLogs && entry.Count > 1)
                ss << " (x" << entry.Count << ")";
        }
        else {
            for (usize i = 0; i < m_Logs.Size(); ++i)
            {
                const auto& entry = m_Logs[i];

                if (entry.Source == LogSource::eEngine && !m_ShowEngine)
                    continue;
                if (entry.Source == LogSource::eClient && !m_ShowClient)
                    continue;

                if (entry.Level == LogLevel::eDebug && !m_ShowDebug) continue;
                if (entry.Level == LogLevel::eInfo && !m_ShowInfo) continue;
                if (entry.Level == LogLevel::eWarn && !m_ShowWarn) continue;
                if (entry.Level == LogLevel::eError && !m_ShowError) continue;
                if (entry.Level == LogLevel::eFatal && !m_ShowFatal) continue;

                String tagKey = entry.Tag.Empty() ? "Untagged" : entry.Tag;
                auto   tagIt  = m_TagFilters.Find(tagKey);
                if (tagIt != m_TagFilters.end() && !tagIt->Value) continue;

                if (!filterStr.Empty())
                {
                    if (entry.Payload.Find(filterStr) == String::NPos
                        && entry.Tag.Find(filterStr) == String::NPos)
                        continue;
                }

                String sourceStr = (entry.Source == LogSource::eEngine)
                                     ? "Engine"
                                     : "Client";
                ss << "[" << entry.Timestamp.Raw() << "] [" << sourceStr.Raw()
                   << "] [" << entry.Tag.Raw() << "] " << entry.Payload.Raw();
                if (m_CollapseLogs && entry.Count > 1)
                    ss << " (x" << entry.Count << ")";
                ss << "\n";
            }
        }

        ImGui::SetClipboardText(ss.str().c_str());
    }

    void ConsolePanel::ExportLogFile(const String& filePath)
    {
        std::ofstream outFile(filePath.Raw());
        if (!outFile.is_open()) return;

        String filterStr = m_FilterBuffer;

        for (usize i = 0; i < m_Logs.Size(); ++i)
        {
            const auto& entry = m_Logs[i];

            if (entry.Source == LogSource::eEngine && !m_ShowEngine) continue;
            if (entry.Source == LogSource::eClient && !m_ShowClient) continue;

            if (entry.Level == LogLevel::eDebug && !m_ShowDebug) continue;
            if (entry.Level == LogLevel::eInfo && !m_ShowInfo) continue;
            if (entry.Level == LogLevel::eWarn && !m_ShowWarn) continue;
            if (entry.Level == LogLevel::eError && !m_ShowError) continue;
            if (entry.Level == LogLevel::eFatal && !m_ShowFatal) continue;

            String tagKey = entry.Tag.Empty() ? "Untagged" : entry.Tag;
            auto   tagIt  = m_TagFilters.Find(tagKey);
            if (tagIt != m_TagFilters.end() && !tagIt->Value) continue;

            if (!filterStr.Empty())
            {
                if (entry.Payload.Find(filterStr) == String::NPos
                    && entry.Tag.Find(filterStr) == String::NPos)
                    continue;
            }

            String sourceStr
                = (entry.Source == LogSource::eEngine) ? "Engine" : "Client";
            outFile << "[" << entry.Timestamp.Raw() << "] [" << sourceStr.Raw()
                    << "] [" << entry.Tag.Raw() << "] " << entry.Payload.Raw();

            if (m_CollapseLogs && entry.Count > 1)
                outFile << " (x" << entry.Count << ")";

            if (!entry.SourceFile.Empty())
                outFile << " (" << entry.SourceFile.Raw() << ":"
                        << entry.SourceLine << ")";

            outFile << "\n";
        }
    }

    void ConsolePanel::OpenFileInIDE(IDEType ide, const String& filePath,
                                     u32 line)
    {
        if (filePath.Empty()) return;

        String command;
        if (ide == IDEType::eVSCode)
            command = "code -g \"" + filePath + ":"
                    + StringUtils::ToString(line) + "\"";
        else if (ide == IDEType::eVisualStudio)
            command = "devenv /Command \"Edit.Goto " + filePath + " "
                    + StringUtils::ToString(line) + "\"";

        command += " &";
        std::system(command.Raw());
    }
} // namespace Vortex::Editor
