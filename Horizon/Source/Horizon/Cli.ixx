/*
 * Created by v1tr10l7 on 21.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <chrono>
#include <cstdio>
#include <thread>

export module Horizon.Cli;

import Horizon.Http;
import Horizon.Installs;
import Horizon.Manifest;
import Horizon.Platform;
import Horizon.Settings;

/*
 * Headless commands, for scripts and for testing a release server without
 * the UI. They run before any window exists:
 *
 *   Horizon --list-releases            what the manifest offers
 *   Horizon --installs                 what's installed
 *   Horizon --install <version|latest> download, verify and unpack
 *   Horizon --manifest <url> ...       use another manifest for this run
 */
export namespace Horizon
{
    /// The exit code when `args` holds a headless command, else nullopt
    /// (start the launcher UI).
    Optional<int> RunCommandLine(const Vector<String>& args);
} // namespace Horizon

namespace Horizon
{
    namespace
    {
        using Arguments = Vector<String>;

        bool Has(const Arguments& args, StringView flag)
        {
            for (const auto& arg : args)
                if (arg.View() == flag) return true;
            return false;
        }
        /// The argument after `flag`; empty if it's missing or last.
        StringView Value(const Arguments& args, StringView flag)
        {
            for (usize i = 0; i + 1 < args.Size(); ++i)
                if (args[i].View() == flag) return args[i + 1].View();
            return {};
        }

        Optional<Manifest> Fetch(const String& url)
        {
            String body, error;
            auto   result = Http::Get(url, body);
            if (!result.Ok)
            {
                std::fprintf(stderr, "horizon: can't download %s: %s\n", url.Raw(),
                             result.Error.Raw());
                return NullOpt;
            }
            auto manifest = ParseManifest(body, url, error);
            if (!manifest) std::fprintf(stderr, "horizon: %s\n", error.Raw());
            return manifest;
        }

        bool IsInstalled(const Vector<EngineInstall>& installs, StringView version)
        {
            for (const auto& install : installs)
                if (!install.Local && install.Version.View() == version) return true;
            return false;
        }

        int ListReleases(const Settings& settings)
        {
            auto manifest = Fetch(settings.ManifestUrl);
            if (!manifest) return 1;
            auto installs = ScanInstalls(settings);
            auto platform = HostPlatform();
            for (const auto& release : manifest->Releases)
            {
                const Artifact* artifact = release.ForPlatform(platform);
                std::printf("%-16s %-9s %-11s %8.1f MB  %s\n", release.Version.Raw(),
                            release.Channel.Raw(), release.Date.Raw(),
                            artifact ? static_cast<double>(artifact->Size) / 1e6 : 0.0,
                            IsInstalled(installs, release.Version) ? "installed"
                            : artifact                             ? ""
                                       : "(no build for this platform)");
            }
            if (manifest->Releases.Empty()) std::printf("The manifest lists no releases.\n");
            return 0;
        }

        int ListInstalls(const Settings& settings)
        {
            auto installs = ScanInstalls(settings);
            for (const auto& install : installs)
                std::printf("%-24s %-9s %s%s\n", install.Id.Raw(), install.Channel.Raw(),
                            String(install.Executable.StrView()).Raw(),
                            install.Id == settings.DefaultEngine ? "  (default)" : "");
            if (installs.Empty())
                std::printf("Nothing installed in %s\n",
                            String(settings.InstallDirectory.StrView()).Raw());
            return 0;
        }

        int Install(Settings& settings, StringView wanted)
        {
            auto manifest = Fetch(settings.ManifestUrl);
            if (!manifest) return 1;

            auto           platform = HostPlatform();
            const Release* release  = nullptr;
            for (const auto& candidate : manifest->Releases)
            {
                bool match = wanted == "latest"
                               ? candidate.IsStable() && candidate.ForPlatform(platform)
                               : candidate.Version.View() == wanted;
                if (match)
                {
                    release = &candidate;
                    break;
                }
            }
            if (!release)
            {
                std::fprintf(stderr, "horizon: no release '%s' in %s\n", String(wanted).Raw(),
                             settings.ManifestUrl.Raw());
                return 1;
            }
            const Artifact* artifact = release->ForPlatform(platform);
            if (!artifact)
            {
                std::fprintf(stderr, "horizon: %s has no build for %s\n",
                             release->Version.Raw(), platform.Raw());
                return 1;
            }
            if (IsInstalled(ScanInstalls(settings), release->Version))
            {
                std::printf("Vortex %s is already installed\n", release->Version.Raw());
                return 0;
            }

            std::printf("Installing Vortex %s into %s\n", release->Version.Raw(),
                        String(settings.InstallDirectory.StrView()).Raw());
            InstallJob job(*release, *artifact, settings.InstallDirectory);
            int        lastPercent = -1;
            auto       lastState   = InstallJob::State::eDownloading;
            while (!job.Finished())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                int percent = static_cast<int>(job.Progress() * 100.0f);
                if (job.CurrentState() == InstallJob::State::eDownloading
                    && percent / 10 != lastPercent / 10)
                {
                    std::printf("  downloading %3d%%  (%.1f MB)\n", percent < 0 ? 0 : percent,
                                static_cast<double>(job.BytesReceived()) / 1e6);
                    lastPercent = percent;
                }
                if (job.CurrentState() != lastState)
                {
                    lastState = job.CurrentState();
                    if (lastState == InstallJob::State::eVerifying)
                        std::printf("  verifying sha256\n");
                    if (lastState == InstallJob::State::eExtracting)
                        std::printf("  unpacking\n");
                }
                std::fflush(stdout);
            }
            if (job.CurrentState() == InstallJob::State::eFailed)
            {
                std::fprintf(stderr, "horizon: %s\n", job.Error().Raw());
                return 1;
            }

            if (settings.DefaultEngine.Empty())
            {
                // Reloaded, so a --manifest override isn't saved with it.
                Settings saved      = Settings::Load();
                saved.DefaultEngine = release->Version;
                String error;
                if (!saved.Save(&error)) std::fprintf(stderr, "horizon: %s\n", error.Raw());
            }
            std::printf("Installed Vortex %s\n", release->Version.Raw());
            return 0;
        }
    } // namespace

    Optional<int> RunCommandLine(const Arguments& args)
    {
        if (Has(args, "--help") || Has(args, "-h"))
        {
            std::printf("Usage: Horizon [--manifest <url>] [command]\n"
                        "  (no command)               open the launcher\n"
                        "  --page <projects|engines|settings|new>  open on that page\n"
                        "                             (new: the New project dialog)\n"
                        "  --list-releases            releases the manifest offers\n"
                        "  --installs                 installed engines\n"
                        "  --install <version|latest> download and install a release\n");
            return 0;
        }

        bool listReleases = Has(args, "--list-releases");
        bool listInstalls = Has(args, "--installs");
        auto install      = Value(args, "--install");
        if (!listReleases && !listInstalls && install.Empty()) return NullOpt;

        Settings settings = Settings::Load();
        if (auto url = Value(args, "--manifest"); !url.Empty()) settings.ManifestUrl = String(url);

        if (listReleases) return ListReleases(settings);
        if (listInstalls) return ListInstalls(settings);
        return Install(settings, install);
    }
} // namespace Horizon
