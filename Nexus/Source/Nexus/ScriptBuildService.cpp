/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <filesystem>
#include <Nexus/ScriptBuildService.hpp>
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Script/ScriptEngine.hpp>

import Prism.Containers.Vector;
import Prism.String.String;

namespace Vortex::Editor
{
    namespace
    {
        /// How often the sources are scanned, and how long they must stay
        /// unchanged before a build starts (an editor saving several files,
        /// or saving twice, gives one build).
        constexpr f32 kScanInterval = 0.5f;
        constexpr f32 kDebounce     = 0.6f;

        constexpr const char* kScriptRoot = "Sandbox";
    } // namespace

    void ScriptBuildService::Build(bool incremental)
    {
        if (IsBuilding())
        {
            // Sources changed under a running build: build again after it.
            m_BuildQueued = true;
            m_Incremental = m_Incremental && incremental;
            return;
        }
        String project
            = (std::filesystem::current_path() / kScriptRoot / "sandbox.csproj")
                  .string()
                  .data();
        Vector<String> arguments = {"build", project, "-c", "Debug"};
        if (!incremental) arguments.PushBack("--no-incremental");
        m_Task = Process::Create("dotnet", arguments);
        if (!m_Task->Start())
        {
            m_State  = ScriptBuildState::eFailed;
            m_Output = "Failed to start dotnet";
            m_Task.Reset();
            VtCoreErrorTag("Editor", "{}", m_Output);
            return;
        }
        m_State       = ScriptBuildState::eBuilding;
        m_Output      = "Building scripts...";
        m_BuildQueued = false;
        m_Incremental = true;
        VtCoreTraceTag("Editor", "Building scripts{}...",
                       incremental ? " (sources changed)" : "");
    }

    void ScriptBuildService::Update(f32 deltaTime, bool autoCompile, bool sceneRunning)
    {
        Collect(sceneRunning);

        // A build that finished in Play takes effect once editing resumes.
        if (m_ReloadPending && !sceneRunning && !IsBuilding()) Reload();

        if (!autoCompile)
        {
            // Pick up from the current state when turned back on.
            m_HaveSnapshot = false;
            m_ChangeAge    = -1.0f;
            return;
        }
        Watch(deltaTime);
    }

    void ScriptBuildService::Collect(bool sceneRunning)
    {
        if (!IsBuilding() || !m_Task) return;
        if (m_Task->Poll() == ProcessState::eRunning) return;

        ProcessResult result = m_Task->Wait();
        m_Task.Reset();
        m_ExitCode = result.exitCode;
        if (result.exitCode != 0)
        {
            m_State  = ScriptBuildState::eFailed;
            // dotnet prints compiler errors on stdout.
            m_Output = result.stdoutOutput.Empty() ? result.stderrOutput
                                                   : result.stdoutOutput;
            VtCoreErrorTag("Editor", "Script build failed ({}); keeping the loaded "
                                     "assembly:\n{}",
                           result.exitCode, m_Output);
        }
        else
        {
            m_State  = ScriptBuildState::eSucceeded;
            m_Output = result.stdoutOutput;
            VtCoreInfoTag("Editor", "Script build succeeded");
            if (sceneRunning)
            {
                m_ReloadPending = true;
                VtCoreInfoTag("Editor", "Scripts reload when the scene stops");
            }
            else Reload();
        }

        if (m_BuildQueued) Build(m_Incremental);
    }

    void ScriptBuildService::Reload()
    {
        m_ReloadPending = false;
        if (!ScriptEngine::ReloadAssembly())
            VtCoreErrorTag("Editor", "Failed to reload the script assembly");
    }

    void ScriptBuildService::Watch(f32 deltaTime)
    {
        if (m_ChangeAge >= 0.0f) m_ChangeAge += deltaTime;

        m_ScanTimer -= deltaTime;
        if (m_ScanTimer <= 0.0f)
        {
            m_ScanTimer   = kScanInterval;
            u64 snapshot  = ScanSources();
            if (!m_HaveSnapshot)
            {
                // The first scan is the baseline, not a change.
                m_Snapshot     = snapshot;
                m_HaveSnapshot = true;
            }
            else if (snapshot != m_Snapshot)
            {
                m_Snapshot  = snapshot;
                m_ChangeAge = 0.0f;
            }
        }

        if (m_ChangeAge >= kDebounce)
        {
            m_ChangeAge = -1.0f;
            Build(/*incremental=*/true);
        }
    }

    u64 ScriptBuildService::ScanSources()
    {
        namespace fs = std::filesystem;
        std::error_code error;
        const fs::path  root = kScriptRoot;
        if (!fs::exists(root, error)) return 0;

        // FNV-1a over every source's path, size and write time: any edit,
        // addition, removal or rename changes it.
        u64  hash = 1469598103934665603ull;
        auto mix  = [&hash](const void* data, usize size)
        {
            const auto* bytes = static_cast<const u8*>(data);
            for (usize i = 0; i < size; ++i)
            {
                hash ^= bytes[i];
                hash *= 1099511628211ull;
            }
        };
        for (fs::recursive_directory_iterator it(root, error), end; it != end;
             it.increment(error))
        {
            if (error) break;
            const auto& path = it->path();
            if (it->is_directory(error))
            {
                // Build output: changes there are the build's own.
                if (path.filename() == "obj" || path.filename() == "bin")
                    it.disable_recursion_pending();
                continue;
            }
            auto extension = path.extension();
            if (extension != ".cs" && extension != ".csproj") continue;

            std::string name = path.generic_string();
            mix(name.data(), name.size());
            u64  size = it->file_size(error);
            auto time = it->last_write_time(error).time_since_epoch().count();
            mix(&size, sizeof(size));
            mix(&time, sizeof(time));
        }
        return hash;
    }
} // namespace Vortex::Editor
