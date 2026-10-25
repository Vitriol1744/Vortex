/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorProject.hpp>

#include <Prism/Containers/Array.hpp>
#include <Prism/String/StringBuilder.hpp>

#include <Vortex/Engine/Application.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Renderer/RHI/Shader.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>

namespace Vortex::Editor
{
    namespace
    {
        String ToKebabCase(StringView input)
        {
            StringBuilder builder;

            // Prevents leading dashes
            bool          prevIsDash = true;

            for (usize i = 0; i < input.Size(); ++i)
            {
                char current = input[i];

                if (CodePoints::IsAlphanumeric(
                        static_cast<unsigned char>(current)))
                {
                    // Detect CamelCase transitions (e.g., "MyProject" ->
                    // "my-project")
                    if (CodePoints::IsUpper(
                            static_cast<unsigned char>(current)))
                    {
                        if (i > 0 && !prevIsDash
                            && CodePoints::IsLower(
                                static_cast<unsigned char>(input[i - 1])))
                            builder << '-';
                        builder << static_cast<char>(CodePoints::ToLower(
                            static_cast<unsigned char>(current)));
                    }
                    else builder << current;
                    prevIsDash = false;
                }
                // Treat spaces, underscores, dashes, and special characters as
                // word separators
                else if (!prevIsDash && !builder.Empty())
                {
                    builder << '-';
                    prevIsDash = true;
                }
            }

            String result = builder;
            // Clean up trailing dash if special characters were at the end
            return (!result.Empty() && result.Back() == '-')
                     ? result.Substr(0, result.Size() - 1)
                     : result;
        }

        String GenerateProjectFileName(StringView projectName)
        {
            return ToKebabCase(projectName) + ".vproj";
        }

        ErrorOr<void> ValidateTemplatePath(PathView templatePath)
        {
            if (templatePath.Empty()) return {};

            if (!Filesystem::Exists(templatePath)) return Error(ENOENT);

            // FIXME(v1tr10l7): Allow specifying paths to .vproj as a template
            // path
            if (!Filesystem::IsDirectory(templatePath)) return Error(ENOTDIR);
            if (Filesystem::IsEmpty(templatePath)) return Error(ENODATA);

            return {};
        }
        ErrorOr<void> ValidateNewProjectPath(PathView path)
        {
            if (path.Empty()) return Error(EINVAL);

            if (Filesystem::Exists(path))
            {
                if (!Filesystem::IsDirectory(path)) return Error(ENOTDIR);
                if (!Filesystem::IsEmpty(path)) return Error(ENOTEMPTY);
            }

            return {};
        }
        namespace fs = std::filesystem;

        bool CopyTemplateDirectory(PathView templateDir, PathView targetDir)
        {
            std::error_code ec;

            if (!fs::exists(targetDir.Raw()))
            {
                fs::create_directories(targetDir.Raw(), ec);
                if (ec) return false;
            }

            const auto copyOptions = fs::copy_options::recursive
                                   | fs::copy_options::overwrite_existing;

            fs::copy(templateDir.Raw(), targetDir.Raw(), copyOptions, ec);

            return !ec;
        }
    }; // namespace

    EditorProject::EditorProject(::Ref<Project> runtimeProject, PathView path)
        : m_RuntimeProject(runtimeProject)
        , m_ProjectPath(path.ParentPath())
        , m_ProjectConfigPath(path)
    {
    }

    void EditorProject::SwitchTo(::Ref<EditorProject> project)
    {
        VtCoreAssert(project);
        const auto& config = project->RuntimeProject()->Config();

        StringView  name   = config.Metadata.Name;
        ::Path      path   = project->Path();
        VtCoreTraceTag("Project", "Switching current scene to '{}'", name);

        auto  scenePath = path / config.StartScenePath;

        auto& scene     = *Scene::Active();
        if (!DeserializeJsonFromFile(scenePath, scene))
            VtCoreErrorTag("Project", "Failed to deserialize scene at {}",
                           scenePath);

        s_Active = project;
        Filesystem::UnmountAll();

        auto mountPoints = ToArray<Array<StringView, 2>>(
            {{"Assets", config.AssetDirectory},
             {"Internal", "assets"},
             {"Scripts", config.AssetDirectory / "Scripts"}});

        for (const auto& mount : mountPoints)
        {
            auto   virt = mount[0];
            ::Path phys = mount[1];
            if (virt != "Internal") phys = path / mount[1];

            VtCoreTraceTag("Project", "Mounting `{}` to `{}`", virt, phys);
            Filesystem::Mount(virt, phys);
        }

        auto& app = *Application::Get();
        app.PublishEvent<ProjectChangedEvent>(project, 0);
    }
    ::Ref<EditorProject> EditorProject::New(const ProjectConfig& config,
                                            PathView             projectPath,
                                            PathView             templatePath)
    {
        VtCoreTraceTag(
            "Project", "Creating new project at '{}' using template "
            "'{}'...",
            projectPath, templatePath);

        auto templateValid = ValidateTemplatePath(templatePath);
        if (!templateValid)
        {
            VtCoreErrorTag("Project", "Template path is not valid -> {}",
                        std::strerror(templateValid.Error()));
            return nullptr;
        }

        auto projectPathValid = ValidateNewProjectPath(projectPath);
        if (!projectPathValid)
        {
            VtCoreErrorTag("Project", "New project's path is invalid -> {}",
                        std::strerror(projectPathValid.Error()));
            return nullptr;
        }

        if (!Filesystem::Exists(projectPath))
        {
            VtCoreTraceTag("Project",
                           "Project's path does not exist, creating "
                           "'{}'...",
                           projectPath);
            Filesystem::CreateDirectories(projectPath);
        }

        auto paths = ToArray<PathView>({
            "Binaries",
            "Cache",
            "Assets/Scenes",
            "Assets/Scripts",
        });

        for (PathView path : paths)
        {
            class Path fullPath = ::Path(projectPath) / path;
            VtCoreTrace("EditorProject: Creating directory '{}'...", fullPath);
            if (!Filesystem::CreateDirectories(fullPath))
            {
                VtCoreErrorTag("Project", "Failed to create directory: {}",
                               fullPath.View());
                return nullptr;
            }
        }

        if (!templatePath.Empty())
            CopyTemplateDirectory(templatePath, projectPath);
        ::Path defaultSceneSource = "assets/scenes/cube.vscene";
        ::Path targetScenePath    = projectPath / config.StartScenePath;

        if (Filesystem::Exists(defaultSceneSource))
        {
            if (auto parentPath = targetScenePath.ParentPath();
                !parentPath.Empty())
                Filesystem::CreateDirectories(parentPath);

            std::error_code ec;
            fs::copy_file(defaultSceneSource.Raw(), targetScenePath.Raw(),
                          fs::copy_options::overwrite_existing, ec);

            if (ec)
            {
                VtCoreErrorTag("Project",
                               "Failed to copy scene template from '{}' to "
                               "'{}': {}",
                               defaultSceneSource.View(),
                               targetScenePath.View(), ec.message().c_str());
            }
            else {
                VtCoreTrace("EditorProject: Copied scene template to '{}'",
                            targetScenePath.View());
            }
        }
        else {
            VtCoreWarnTag("Project",
                          "Default scene template '{}' does not exist!",
                          defaultSceneSource.View());
        }

        StringView projectName = config.Metadata.Name;
        auto       configFilePath
            = ::Path(projectPath) / GenerateProjectFileName(projectName);
        SerializeJsonToFile(configFilePath, config);

        auto runtimeProject = Project::Load(configFilePath);
        if (!runtimeProject)
        {
            VtCoreErrorTag("Project", "Failed to load the project from => '{}'",
                           configFilePath);
            return nullptr;
        }

        return CreateRef<EditorProject>(runtimeProject, configFilePath);
    }
    ::Ref<EditorProject> EditorProject::Load(PathView projectConfigPath)
    {
        auto projectPath = projectConfigPath.ParentPath();
        VtCoreTraceTag("Project", "Loading project at '{}'...", projectPath);

        auto runtimeProject = Project::Load(projectConfigPath);
        if (!runtimeProject)
        {
            VtCoreErrorTag("Project",
                           "Failed to load project from file\nprojectPath "
                           "-> {}",
                           projectConfigPath);
            return nullptr;
        }

        return CreateRef<EditorProject>(runtimeProject, projectConfigPath);
    }
}; // namespace Vortex::Editor
