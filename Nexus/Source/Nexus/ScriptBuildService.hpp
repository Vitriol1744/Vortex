/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Process.hpp>

import Prism.Memory.Scope;
import Prism.String.String;

namespace Vortex::Editor
{
    enum class ScriptBuildState
    {
        eIdle,
        eBuilding,
        eSucceeded,
        eFailed,
    };

    /// Builds the project's C# scripts in the background (dotnet build) and
    /// hot-reloads the assembly when it succeeds. Optionally watches the
    /// sources and rebuilds on change.
    ///
    /// The assembly is only swapped while editing: a build that finishes in
    /// Play is reloaded when the scene stops, since a reload destroys every
    /// script instance. A failed build keeps the loaded assembly.
    class ScriptBuildService
    {
      public:
        /// Starts a build; while one runs, queues another. `incremental`
        /// lets dotnet skip what didn't change (the watcher's builds).
        void                    Build(bool incremental = false);
        /// Every frame: collects a finished build, reloads when allowed and,
        /// with `autoCompile`, watches the sources.
        void                    Update(f32 deltaTime, bool autoCompile, bool sceneRunning);

        inline ScriptBuildState State() const { return m_State; }
        inline bool IsBuilding() const { return m_State == ScriptBuildState::eBuilding; }
        inline const String&    Output() const { return m_Output; }
        inline isize            ExitCode() const { return m_ExitCode; }
        /// A build succeeded during Play; reloaded when the scene stops.
        inline bool             IsReloadPending() const { return m_ReloadPending; }
        /// Sources changed and a build is about to start (debouncing).
        inline bool             HasPendingChanges() const { return m_ChangeAge >= 0.0f; }

      private:
        Scope<Process>   m_Task     = nullptr;
        ScriptBuildState m_State    = ScriptBuildState::eIdle;
        String           m_Output{};
        isize            m_ExitCode = 0;

        bool             m_ReloadPending = false;
        bool             m_BuildQueued   = false;
        bool             m_Incremental   = false;

        // Source watching.
        bool             m_HaveSnapshot  = false;
        u64              m_Snapshot      = 0;
        f32              m_ScanTimer     = 0.0f;
        /// Seconds since the last change seen; negative when none pending.
        f32              m_ChangeAge     = -1.0f;

        void             Collect(bool sceneRunning);
        void             Reload();
        void             Watch(f32 deltaTime);
        /// A fingerprint of the script sources (paths, sizes, times).
        static u64       ScanSources();
    };
} // namespace Vortex::Editor
