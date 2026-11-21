/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

module Horizon.Installs;

import Prism.Algorithm.Sort;
import Prism.String.Formatter;
import Horizon.Http;
import Horizon.Json;
import Horizon.Sha256;

namespace Horizon
{
    namespace
    {
        constexpr const char* kMarkerFile = "horizon-install.json";

        /// A dev tree runs Nexus from <root>: bin/Nexus (Debug) and
        /// bin/<Config>/Nexus both load <root>/assets.
        Path GuessWorkingDirectory(const Path& executable)
        {
            Path dir = executable.ParentPath();
            Path up  = dir.ParentPath();
            for (int level = 0; level < 2 && !up.Empty(); ++level, up = up.ParentPath())
                if (Exists(up / "assets")) return up;
            return dir;
        }

        String ExecutableFor(const Artifact& artifact)
        {
            return artifact.Executable.Empty() ? EditorExecutableName() : artifact.Executable;
        }

        void WriteMarker(const Path& root, const Release& release, const Artifact& artifact)
        {
            using Json::From;
            Json::Value marker = {
                {"Version", From(release.Version)},
                {"Channel", From(release.Channel)},
                {"Nexus", From(release.NexusVersion)},
                {"Vortex", From(release.VortexVersion)},
                {"Notes", From(release.Notes)},
                {"Executable", From(ExecutableFor(artifact))},
                {"Source", From(artifact.Url)},
            };
            std::ofstream(ToFs(root / kMarkerFile)) << marker.dump(2) << '\n';
        }

        Optional<EngineInstall> ReadMarker(const Path& root)
        {
            Json::Value marker = Json::ReadFile(root / kMarkerFile);
            if (marker.is_discarded() || !marker.is_object()) return NullOpt;

            EngineInstall install;
            install.Version          = Json::GetString(marker, "Version", root.BaseName());
            install.Id               = install.Version;
            install.Channel          = Json::GetString(marker, "Channel");
            install.DisplayName      = Format("Vortex {}", install.Version);
            install.Root             = root;
            install.Executable       = root / Path(Json::GetString(marker, "Executable",
                                                                   EditorExecutableName()));
            install.WorkingDirectory = root;
            install.LibraryDirectory = root / "lib";
            return install;
        }
    } // namespace

    Vector<EngineInstall> ScanInstalls(const Settings& settings)
    {
        Vector<EngineInstall> installs;
        std::error_code       ec;
        for (const auto& entry : fs::directory_iterator(ToFs(settings.InstallDirectory), ec))
        {
            // .staging-* directories are unfinished installs.
            if (!entry.is_directory() || entry.path().filename().string().starts_with("."))
                continue;
            if (auto install = ReadMarker(FromFs(entry.path()))) installs.PushBack(*install);
        }
        Prism::Sort(installs.begin(), installs.end(), [](const auto& a, const auto& b)
             { return CompareVersions(a.Version, b.Version) > 0; });

        for (const auto& build : settings.LocalBuilds)
        {
            EngineInstall install;
            install.Id               = Format("local:{}", build.Name);
            install.DisplayName      = build.Name;
            install.Version          = "local";
            install.Channel          = "local";
            install.Executable       = build.Executable;
            install.Root             = build.Executable.ParentPath();
            install.WorkingDirectory = build.WorkingDirectory.Empty()
                                         ? GuessWorkingDirectory(build.Executable)
                                         : build.WorkingDirectory;
            install.Local            = true;
            installs.PushBack(Move(install));
        }
        return installs;
    }

    const EngineInstall* PickInstall(const Vector<EngineInstall>& installs, StringView id,
                                     const Settings& settings)
    {
        for (StringView wanted : {id, settings.DefaultEngine.View()})
        {
            if (wanted.Empty()) continue;
            for (const auto& install : installs)
                if (install.Id.View() == wanted) return &install;
        }
        return installs.Empty() ? nullptr : &installs.Front();
    }

    bool RemoveInstall(const EngineInstall& install, String& error)
    {
        if (install.Local || !Exists(install.Root / kMarkerFile))
        {
            error = Format("Not a Horizon install: {}", install.Root);
            return false;
        }
        std::error_code ec;
        fs::remove_all(ToFs(install.Root), ec);
        if (ec) error = Format("Can't remove {}: {}", install.Root, ec.message().c_str());
        return !ec;
    }

    InstallJob::InstallJob(Release release, Artifact artifact, Path installDirectory)
        : m_Release(Move(release))
        , m_Artifact(Move(artifact))
        , m_InstallDirectory(Move(installDirectory))
    {
        m_Total  = m_Artifact.Size;
        m_Thread = std::thread([this] { Run(); });
    }
    InstallJob::~InstallJob()
    {
        m_Cancel = true;
        if (m_Thread.joinable()) m_Thread.join();
    }

    float InstallJob::Progress() const
    {
        u64 total = m_Total.Load();
        if (total == 0) return -1.0f;
        float progress = static_cast<float>(m_Received.Load()) / static_cast<float>(total);
        return progress > 1.0f ? 1.0f : progress;
    }
    String InstallJob::Error() const
    {
        std::lock_guard lock(m_ErrorMutex);
        return m_Error;
    }
    void InstallJob::Fail(String error)
    {
        {
            std::lock_guard lock(m_ErrorMutex);
            m_Error = Move(error);
        }
        m_State = State::eFailed;
    }

    void InstallJob::Run()
    {
        std::error_code ec;
        const String&   version = m_Release.Version;
        const Path      target  = m_InstallDirectory / Path(version);
        const Path      staging = m_InstallDirectory / Path(Format(".staging-{}", version));
        const Path      archive = m_InstallDirectory / Path(Format(".download-{}.tar.gz", version));

        fs::create_directories(ToFs(m_InstallDirectory), ec);
        if (ec)
            return Fail(Format("Can't create {}: {}", m_InstallDirectory, ec.message().c_str()));
        if (Exists(target)) return Fail(Format("Version {} is already installed", version));

        // 1. Download.
        auto result = Http::Download(
            m_Artifact.Url, archive,
            [this](u64 now, u64 total)
            {
                m_Received = now;
                if (total) m_Total = total;
            },
            &m_Cancel);
        if (!result.Ok) return Fail(Format("Download failed: {}", result.Error));

        // 2. Verify. An artifact without a hash is only accepted from a
        // local file:// manifest, where there's nothing to tamper with.
        m_State = State::eVerifying;
        if (!m_Artifact.Sha256.Empty())
        {
            auto digest = FileSha256(archive);
            if (!digest || *digest != m_Artifact.Sha256)
            {
                fs::remove(ToFs(archive), ec);
                return Fail(Format("Checksum mismatch: expected {}, got {}", m_Artifact.Sha256,
                                   digest ? StringView(*digest) : StringView("<unreadable>")));
            }
        }
        else if (!m_Artifact.Url.StartsWith("file://"))
        {
            fs::remove(ToFs(archive), ec);
            return Fail("The manifest gives no sha256 for this download");
        }
        if (m_Cancel.Load())
        {
            fs::remove(ToFs(archive), ec);
            return Fail("Cancelled");
        }

        // 3. Unpack into a staging directory, then move it into place, so
        // an interrupted install never looks installed.
        m_State = State::eExtracting;
        fs::remove_all(ToFs(staging), ec);
        fs::create_directories(ToFs(staging), ec);
        String output;
        int    code = RunAndWait({"tar", "-xzf", String(archive.StrView()), "-C",
                                  String(staging.StrView())},
                                 &output);
        fs::remove(ToFs(archive), ec);
        if (code != 0)
        {
            fs::remove_all(ToFs(staging), ec);
            return Fail(
                Format("Can't unpack the release (tar exited with {}): {}", code, output));
        }

        // Archives hold one top-level directory (vortex-<version>/).
        Path         root = staging;
        Vector<Path> entries;
        for (const auto& entry : fs::directory_iterator(ToFs(staging), ec))
            entries.PushBack(FromFs(entry.path()));
        if (entries.Size() == 1 && IsDirectory(entries.Front())) root = entries.Front();

        Path executable = root / Path(ExecutableFor(m_Artifact));
        if (!Exists(executable))
        {
            fs::remove_all(ToFs(staging), ec);
            return Fail(Format("The release has no {}", executable.BaseName()));
        }

        WriteMarker(root, m_Release, m_Artifact);
        fs::rename(ToFs(root), ToFs(target), ec);
        if (ec)
        {
            fs::remove_all(ToFs(staging), ec);
            return Fail(Format("Can't move the release into {}: {}", target,
                               ec.message().c_str()));
        }
        fs::remove_all(ToFs(staging), ec);
        m_State = State::eDone;
    }
} // namespace Horizon
