/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

export module Horizon.Projects;

import Horizon.Installs;
import Horizon.Platform;
import Horizon.Settings;

export namespace Horizon
{
    struct ProjectInfo
    {
        String Name;
        Path   File;
        Path   Directory;
    };

    /// Reads a .vproj, or finds the .vproj in a project directory.
    Optional<ProjectInfo> ReadProject(const Path& path);

    /// Starts Nexus on an existing project. Returns an error, or empty.
    String                OpenInEditor(const EngineInstall& engine, const ProjectInfo& project,
                                       const Settings& settings);

    /// Starts Nexus to create a project in `directory` from one of the
    /// engine's templates (empty = its default). Nexus lays out the files
    /// itself, so every engine version creates projects its own way.
    String                CreateInEditor(const EngineInstall& engine, const Path& directory,
                                         StringView name, StringView templateId,
                                         const Settings& settings);

    /// Moves (or adds) the project to the top of the recent list.
    void   RememberProject(Settings& settings, StringView name, const Path& path,
                           StringView engineId);

    /// Why a new project can't go in `directory`; empty if it can.
    String ValidateNewProjectDirectory(const Path& directory);
} // namespace Horizon
