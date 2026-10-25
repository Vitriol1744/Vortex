/*
 * Created by v1tr10l7 on 15.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Nexus/EditorProject.hpp>

namespace Vortex::Editor
{
    struct RecentProject
    {
        String Name;
        Path   Path;
        time_t LastOpened;

        void   Serialize(ArchiveWriter& archive) const;
        bool   Deserialize(ArchiveReader& archive);
    };
    struct EditorPreferences
    {
        Path                                                  StartupProject;
        std::map<time_t, RecentProject, std::greater<time_t>> RecentProjects;

        void InsertRecentProject(const RecentProject& recent);

        void Serialize(ArchiveWriter& archive) const;
        bool Deserialize(ArchiveReader& archive);
    };
}; // namespace Vortex::Editor
