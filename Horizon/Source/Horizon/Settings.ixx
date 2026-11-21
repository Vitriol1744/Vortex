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

export module Horizon.Settings;

import Prism.String.Formatter;
import Horizon.Json;
import Horizon.Platform;

export namespace Horizon
{
    /// An editor build Horizon didn't install, e.g. this repo's bin/Nexus.
    struct LocalBuild
    {
        String Name;
        Path   Executable;
        /// Where assets/ and DotNet/ are; empty = the executable's parent's
        /// parent if that has assets/, else the executable's directory.
        Path   WorkingDirectory;
    };

    struct RecentProject
    {
        String Name;
        /// The .vproj file, or the project directory when Nexus created the
        /// project and named the file itself.
        Path   Location;
        i64    LastOpened = 0;
        /// The engine it was last opened with; empty = the default.
        String Engine;
    };

    struct Settings
    {
        String                ManifestUrl = "http://localhost:8000/manifest.json";
        Path                  InstallDirectory;
        Path                  ProjectsDirectory;
        /// An install's version, or "local:<name>".
        String                DefaultEngine;
        bool                  CloseAfterLaunch = false;
        bool                  ShowPrereleases  = true;
        /// Extra Nexus arguments, split on spaces.
        String                ExtraArguments;
        Vector<LocalBuild>    LocalBuilds;
        Vector<RecentProject> RecentProjects;

        static Path           FilePath() { return ConfigDirectory() / "horizon.json"; }

        /// Defaults filled in for anything the file doesn't set.
        static Settings       Load();
        bool                  Save(String* error = nullptr) const;
    };
} // namespace Horizon

namespace Horizon
{
    Settings Settings::Load()
    {
        Settings settings;
        settings.InstallDirectory  = DataDirectory() / "engines";
        settings.ProjectsDirectory = HomeDirectory() / "VortexProjects";

        // A launcher started from a dev tree (bin/Horizon next to bin/Nexus)
        // offers that build without any setup.
        Path devNexus = ExecutableDirectory() / Path(EditorExecutableName());
        if (Exists(devNexus)) settings.LocalBuilds.PushBack({"Development build", devNexus, {}});

        Json::Value root = Json::ReadFile(FilePath());
        if (root.is_discarded() || !root.is_object()) return settings;

        settings.ManifestUrl       = Json::GetString(root, "ManifestUrl", settings.ManifestUrl);
        settings.InstallDirectory  = Json::GetPath(root, "InstallDirectory", settings.InstallDirectory);
        settings.ProjectsDirectory = Json::GetPath(root, "ProjectsDirectory", settings.ProjectsDirectory);
        settings.DefaultEngine     = Json::GetString(root, "DefaultEngine");
        settings.CloseAfterLaunch  = Json::Get(root, "CloseAfterLaunch", false);
        settings.ShowPrereleases   = Json::Get(root, "ShowPrereleases", true);
        settings.ExtraArguments    = Json::GetString(root, "ExtraArguments");

        if (root.contains("LocalBuilds"))
        {
            settings.LocalBuilds.Clear();
            for (const auto& entry : Json::GetArray(root, "LocalBuilds"))
                settings.LocalBuilds.PushBack({Json::GetString(entry, "Name", "Local build"),
                                               Json::GetPath(entry, "Executable"),
                                               Json::GetPath(entry, "WorkingDirectory")});
        }
        for (const auto& entry : Json::GetArray(root, "RecentProjects"))
            settings.RecentProjects.PushBack({Json::GetString(entry, "Name"),
                                              Json::GetPath(entry, "Path"),
                                              Json::Get(entry, "LastOpened", i64{0}),
                                              Json::GetString(entry, "Engine")});
        return settings;
    }

    bool Settings::Save(String* error) const
    {
        using Json::From;
        Json::Value root;
        root["ManifestUrl"]       = From(ManifestUrl);
        root["InstallDirectory"]  = From(InstallDirectory.StrView());
        root["ProjectsDirectory"] = From(ProjectsDirectory.StrView());
        root["DefaultEngine"]     = From(DefaultEngine);
        root["CloseAfterLaunch"]  = CloseAfterLaunch;
        root["ShowPrereleases"]   = ShowPrereleases;
        root["ExtraArguments"]    = From(ExtraArguments);

        root["LocalBuilds"]       = Json::Value::array();
        for (const auto& build : LocalBuilds)
            root["LocalBuilds"].push_back({{"Name", From(build.Name)},
                                           {"Executable", From(build.Executable.StrView())},
                                           {"WorkingDirectory",
                                            From(build.WorkingDirectory.StrView())}});
        root["RecentProjects"] = Json::Value::array();
        for (const auto& recent : RecentProjects)
            root["RecentProjects"].push_back({{"Name", From(recent.Name)},
                                              {"Path", From(recent.Location.StrView())},
                                              {"LastOpened", recent.LastOpened},
                                              {"Engine", From(recent.Engine)}});

        // Written next to the old file and renamed over it, so a crash
        // can't leave a truncated horizon.json.
        std::error_code ec;
        fs::create_directories(ToFs(ConfigDirectory()), ec);
        Path temporary = Path(Format("{}.tmp", FilePath()));
        {
            std::ofstream file(ToFs(temporary), std::ios::trunc);
            file << root.dump(2) << '\n';
            if (!file)
            {
                if (error) *error = Format("Can't write {}", temporary);
                return false;
            }
        }
        fs::rename(ToFs(temporary), ToFs(FilePath()), ec);
        if (ec && error) *error = Format("Can't replace {}: {}", FilePath(), ec.message().c_str());
        return !ec;
    }
} // namespace Horizon
