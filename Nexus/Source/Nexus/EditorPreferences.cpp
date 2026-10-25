/*
 * Created by v1tr10l7 on 15.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorPreferences.hpp>

namespace Vortex::Editor
{
    inline bool DeserializeValue(
        ArchiveReader& archive, StringView name,
        std::map<time_t, RecentProject, std::greater<time_t>>& projects)
    {
        usize entryCount = 0;
        if (!archive.BeginArray(name, entryCount)) return false;

        for (usize i = 0; i < entryCount; i++)
        {
            RecentProject recent;
            if (!DeserializeValue(archive, "", recent)) continue;

            projects[recent.LastOpened] = recent;
        }

        archive.EndArray();
        return true;
    }

    void RecentProject::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("Name", Name.View());
        archive.Write("Path", Path.View());
        archive.Write("LastOpened", LastOpened);
    }
    bool RecentProject::Deserialize(ArchiveReader& archive)
    {
        archive.Read("Name", Name);
        archive.Read("Path", Path);
        archive.Read("LastOpened", LastOpened);

        return true;
    }

    void EditorPreferences::InsertRecentProject(const RecentProject& recent)
    {
        RecentProjects[recent.LastOpened] = recent;
        StartupProject                    = recent.Name;
    }

    void EditorPreferences::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("StartupProject", StartupProject.View());

        archive.BeginArray("RecentProjects", 10);
        for (usize index = 0; const auto& project : RecentProjects)
        {
            if (index >= 10) break;

            SerializeValue(archive, "", project.second);
            ++index;
        }
        archive.EndArray();
    }
    bool EditorPreferences::Deserialize(ArchiveReader& archive)
    {
        bool result = archive.Read("StartupProject", StartupProject);
        result      = result
              && DeserializeValue(archive, "RecentProjects", RecentProjects);

        return result;
    }
}; // namespace Vortex::Editor
