/*
 * Created by v1tr10l7 on 06.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Project/Project.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>

#include <fstream>

namespace Vortex
{
    void ProjectMetadata::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("Name", Name.View());
        archive.Write("Author", Author.View());
        archive.Write("Description", Description.View());
        SerializeValue(archive, "Version", Version);
    }
    bool ProjectMetadata::Deserialize(ArchiveReader& archive)
    {
        archive.Read("Author", Author);
        archive.Read("Description", Description);
        archive.Read("Name", Name);
        if (!DeserializeValue(archive, "Version", Version)) return false;
        ;

        return true;
    }
    void RendererSettings::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("TargetFrameRate", TargetFrameRate);
        archive.Write("VSync", VSync);
        SerializeValue(archive, "Resolution", Resolution);
    }
    bool RendererSettings::Deserialize(ArchiveReader& archive)
    {
        archive.Read("TargetFrameRate", TargetFrameRate);
        archive.Read("VSync", VSync);
        if (!DeserializeValue(archive, "Resolution", Resolution)) return false;
        return true;
    }

    void ProjectConfig::Serialize(ArchiveWriter& archive) const
    {
        SerializeValue(archive, "Metadata", Metadata);
        SerializeValue(archive, "RendererSettings", RendererSettings);
        archive.Write("StartScenePath", StartScenePath.View());
        archive.Write("AssetDirectory", AssetDirectory.View());
        archive.Write("ScriptModulePath", ScriptModulePath.View());
    }
    bool ProjectConfig::Deserialize(ArchiveReader& archive)
    {
        archive.Read("AssetDirectory", AssetDirectory);
        DeserializeValue(archive, "Metadata", Metadata);
        archive.Read("StartScenePath", StartScenePath);
        archive.Read("ScriptModulePath", ScriptModulePath);

        return true;
    }

    ::Ref<Project> Project::Load(PathView projectPath)
    {
        VtCoreTrace("Project: Loading project at '{}'...", projectPath);
        if (!Filesystem::Exists(projectPath))
        {
            VtCoreError("Project: Invalid project invalid -> '{}'",
                        projectPath);
            return nullptr;
        }

        // TODO(v1tr10l7): Might be a good idea to allow projectPath to be a
        // directory, and automatically load vproj inside
        class Path projectFilePath = projectPath;
        if (Filesystem::IsDirectory(projectFilePath))
        {
            VtCoreError(
                "Project: Project path should be .vproj file, and not a "
                "directory\nprojectPath -> '{}'",
                projectFilePath);
            return nullptr;
        }

        ProjectConfig newConfig;
        if (!DeserializeJsonFromFile(projectFilePath, newConfig))
        {
            VtCoreError(
                "Project: Failed to deserialize the project file -> '{}'",
                projectFilePath);
            return nullptr;
        }
        return CreateRef<Project>(projectFilePath.ParentPath(), newConfig);
    }
}; // namespace Vortex
