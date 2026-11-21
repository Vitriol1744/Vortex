/*
 * Created by v1tr10l7 on 25.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorContext.hpp>
import Prism.Containers.UnorderedMap;
import Prism.Debug.LogSink;
import Prism.Memory.Scope;
import Prism.Memory.WeakRef;
import Prism.String.String;
#include <Vortex/Core/Log/Log.hpp>

#include <print>

namespace Vortex::Editor
{
    enum class IDEType : u8
    {
        eVSCode,
        eVisualStudio
    };
    enum class LogSource : u8
    {
        eEngine,
        eClient,
    };

    struct LogEntry
    {
        String          Timestamp;
        Prism::LogLevel Level;
        String          Tag;
        String          Payload;
        String          SourceFile;
        u32             SourceLine;
        u32             Count  = 1;
        LogSource       Source = LogSource::eEngine;
    };

    class ConsoleSink;
    /// The engine's and the game's log, filterable, with a detail pane.
    class ConsoleWindow final : public EditorWindow,
                                public Weakable<ConsoleWindow>
    {
        friend class ConsoleSink;

      public:
        ConsoleWindow()
            : EditorWindow("Console")
        {
        }

        /// Starts receiving the log from the editor's sink.
        void OnAttach(EditorContext& context) override;
        void OnDraw(EditorContext& context) override;

      private:
        void        PopulateSampleLogs();
        void        RenderToolbar();
        void        UpdateAvailableTags();
        void        RenderCategoryFilter();
        void        RenderSourceFilter();
        void        RenderLogLevelFilter();
        void        RenderLogList(float height);
        void        RenderDetailInspector();

        void        AddLog(LogEntry entry);

        void        CopyToClipboard();
        void        ExportLogFile(const String& filePath);

      public:
        /// Opens `filePath` at `line` in the IDE (in the background).
        static void OpenFileInIDE(IDEType ide, const String& filePath,
                                  u32 line);

      private:
        Vector<LogEntry>           m_Logs;
        /// m_Logs[0, m_TagsScanned) have had their tags registered.
        usize                      m_TagsScanned       = 0;
        /// Indices of the entries passing the filters, rebuilt per frame.
        Vector<u32>                m_Visible;
        i32                        m_SelectedIndex     = -1;

        // Filter States
        char                       m_FilterBuffer[256] = "";
        bool                       m_ShowDebug         = true;
        bool                       m_ShowInfo          = true;
        bool                       m_ShowWarn          = true;
        bool                       m_ShowError         = true;
        bool                       m_ShowFatal         = true;
        bool                       m_ShowEngine        = true;
        bool                       m_ShowClient        = true;
        bool                       m_AutoScroll        = true;
        bool                       m_CollapseLogs      = true;

        UnorderedMap<String, bool> m_TagFilters;
        bool                       m_SelectAllTags      = true;
        bool                       m_SelectAllSources   = true;
        bool                       m_SelectAllLogLevels = true;

        // Detail Inspector Pane
        float                      m_DetailPaneHeight   = 120.0f;
        IDEType                    m_PreferredIDE       = IDEType::eVSCode;
    };

    class ConsoleSink final : public LogSink<Log::MutexPolicy>
    {
      public:
        explicit ConsoleSink(::Ref<ConsoleWindow> window = nullptr)
            : m_Target(window)
        {
        }
        virtual ~ConsoleSink() override = default;

        inline void SetTarget(::Ref<ConsoleWindow> window)
        {
            std::lock_guard<std::mutex> lock(m_BufferMutex);
            m_Target = window;
        }

        /// The sink every console window reads; created at startup and
        /// added to the engine's and the client's loggers.
        static ::Ref<ConsoleSink>& Instance();

        virtual isize SinkIt(const LogMessage& message) override
        {
            if (message.Tag.Empty()) return 0;

            LogEntry entry;
            entry.Timestamp  = TimeToString(message.Timestamp);
            entry.Level      = message.Level;
            entry.Tag        = message.Tag;
            entry.Payload    = message.Payload;
            entry.SourceFile = message.SourceFile;
            entry.SourceLine = message.SourceLine;
            entry.Source     = (message.LoggerName == "Client")
                                 ? LogSource::eClient
                                 : LogSource::eEngine;
            entry.Count      = 1;

            {
                std::lock_guard<std::mutex> lock(m_BufferMutex);
                m_PendingLogs.PushBack(entry);
            }
            return static_cast<isize>(message.Payload.Size());
        }

        virtual void FlushIt() override
        {
            Vector<LogEntry> logsToProcess;

            {
                std::lock_guard<std::mutex> lock(m_BufferMutex);
                if (m_PendingLogs.Empty()) return;

                logsToProcess = Move(m_PendingLogs);
                m_PendingLogs.Clear();
            }

            if (auto window = m_Target.Promote())
            {
                for (usize i = 0; i < logsToProcess.Size(); ++i)
                    window->AddLog(logsToProcess[i]);
            }
        }

      private:
        ::WeakRef<ConsoleWindow> m_Target = nullptr;

        std::mutex              m_BufferMutex;
        Vector<LogEntry>        m_PendingLogs;

        static String           TimeToString(Prism::Time::Timestep timestamp)
        {
            return Prism::Time::Timestep::ToTimeString(timestamp);
        }
    };
} // namespace Vortex::Editor
