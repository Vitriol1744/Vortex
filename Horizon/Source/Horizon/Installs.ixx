/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <mutex>
#include <thread>

export module Horizon.Installs;

import Prism.Utility.Atomic;
import Horizon.Manifest;
import Horizon.Platform;
import Horizon.Settings;

export namespace Horizon
{
    /// An editor Horizon can launch: one it installed, or a local build.
    struct EngineInstall
    {
        /// The version for installs; "local:<name>" for local builds.
        String Id;
        String DisplayName;
        String Version;
        String Channel;
        Path   Root;
        Path   Executable;
        Path   WorkingDirectory;
        /// Holds the shared libraries of a packaged release (lib/).
        Path   LibraryDirectory;
        bool   Local = false;
    };

    /// Installs under Settings::InstallDirectory (each one a directory
    /// holding horizon-install.json), then the local builds; newest first.
    Vector<EngineInstall> ScanInstalls(const Settings& settings);

    /// The install `id` names, else the default engine, else the newest.
    const EngineInstall*  PickInstall(const Vector<EngineInstall>& installs, StringView id,
                                      const Settings& settings);

    /// Deletes an installed release (never a local build).
    bool                  RemoveInstall(const EngineInstall& install, String& error);

    /// Downloads, verifies and unpacks one release on a worker thread.
    class InstallJob
    {
      public:
        enum class State
        {
            eDownloading,
            eVerifying,
            eExtracting,
            eDone,
            eFailed,
        };

        InstallJob(Release release, Artifact artifact, Path installDirectory);
        ~InstallJob();

        InstallJob(const InstallJob&)            = delete;
        InstallJob& operator=(const InstallJob&) = delete;

        void                  Cancel() { m_Cancel = true; }

        const Release&        Target() const { return m_Release; }
        State                 CurrentState() const { return m_State.Load(); }
        bool                  Finished() const
        {
            State state = m_State.Load();
            return state == State::eDone || state == State::eFailed;
        }
        /// 0..1, or a negative value while the size is unknown.
        float                 Progress() const;
        u64                   BytesReceived() const { return m_Received.Load(); }
        /// The size being downloaded; 0 while unknown.
        u64                   BytesTotal() const { return m_Total.Load(); }
        String                Error() const;

      private:
        Release            m_Release;
        Artifact           m_Artifact;
        Path               m_InstallDirectory;

        Atomic<State>      m_State{State::eDownloading};
        Atomic<bool>       m_Cancel{false};
        Atomic<u64>        m_Received{0};
        Atomic<u64>        m_Total{0};
        // std::mutex/std::thread: Prism has no thread, and its Mutex spins.
        mutable std::mutex m_ErrorMutex;
        String             m_Error;
        std::thread        m_Thread;

        void               Run();
        void               Fail(String error);
    };
} // namespace Horizon
