/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <nlohmann/json.hpp>

#include <ctime>
#include <filesystem>

module Horizon.Projects;

import Prism.Algorithm.Find;
import Prism.String.Formatter;
import Horizon.Json;

namespace Horizon
{
    namespace
    {
        String NameFromProjectFile(const Path& file)
        {
            // Nexus writes {"Project": {"Metadata": {"Metadata": {...}}}}.
            Json::Value root     = Json::ReadFile(file);
            const auto& project  = Json::GetObject(root, "Project");
            const auto* metadata = &Json::GetObject(project, "Metadata");
            if (metadata->contains("Metadata")) metadata = &Json::GetObject(*metadata, "Metadata");
            String name = Json::GetString(*metadata, "Name");
            return name.Empty() ? String(Stem(file)) : name;
        }

        Vector<String> SplitArguments(StringView text)
        {
            Vector<String> args;
            usize          start = 0;
            for (usize i = 0; i <= text.Size(); ++i)
            {
                bool space = i == text.Size() || text[i] == ' ' || text[i] == '\t';
                if (!space) continue;
                if (i > start) args.PushBack(String(text.Substr(start, i - start)));
                start = i + 1;
            }
            return args;
        }

        String Launch(const EngineInstall& engine, Vector<String> args, const Settings& settings)
        {
            if (!Exists(engine.Executable))
                return Format("The editor isn't at {}", engine.Executable);

            for (auto& extra : SplitArguments(settings.ExtraArguments)) args.PushBack(Move(extra));

            // Each launch logs to <data>/logs/nexus-<time>.log, since a
            // detached editor has no terminal.
            std::error_code ec;
            Path            logs = DataDirectory() / "logs";
            fs::create_directories(ToFs(logs), ec);

            LaunchRequest request;
            request.Executable       = engine.Executable;
            request.Arguments        = Move(args);
            request.WorkingDirectory = engine.WorkingDirectory;
            request.LibraryDirectory = engine.Local ? Path() : engine.LibraryDirectory;
            if (!ec)
                request.LogFile = logs
                                / Path(Format("nexus-{}.log", static_cast<i64>(std::time(nullptr))));
            return SpawnDetached(request);
        }

        Path Canonical(const Path& path)
        {
            std::error_code ec;
            auto            canonical = fs::weakly_canonical(ToFs(path), ec);
            return ec ? path : FromFs(canonical);
        }
    } // namespace

    Optional<ProjectInfo> ReadProject(const Path& path)
    {
        Path file = path;
        if (IsDirectory(path))
        {
            file = Path();
            std::error_code ec;
            for (const auto& entry : fs::directory_iterator(ToFs(path), ec))
                if (entry.path().extension() == ".vproj")
                {
                    file = FromFs(entry.path());
                    break;
                }
            if (file.Empty()) return NullOpt;
        }
        std::error_code ec;
        if (file.Extension() != "vproj" || !fs::is_regular_file(ToFs(file), ec)) return NullOpt;

        auto absolute = fs::absolute(ToFs(file), ec);
        if (!ec) file = FromFs(absolute);
        return ProjectInfo{NameFromProjectFile(file), file, file.ParentPath()};
    }

    String OpenInEditor(const EngineInstall& engine, const ProjectInfo& project,
                        const Settings& settings)
    {
        return Launch(engine, {"--project", String(project.File.StrView())}, settings);
    }

    String CreateInEditor(const EngineInstall& engine, const Path& directory, StringView name,
                          StringView templateId, const Settings& settings)
    {
        if (auto error = ValidateNewProjectDirectory(directory); !error.Empty()) return error;
        Vector<String> args{String("--new-project"), String(directory.StrView()),
                            String("--name"), String(name)};
        if (!templateId.Empty())
        {
            args.PushBack(String("--template"));
            args.PushBack(String(templateId));
        }
        return Launch(engine, Move(args), settings);
    }

    void RememberProject(Settings& settings, StringView name, const Path& path,
                         StringView engineId)
    {
        Path  key    = Canonical(path);
        auto& recent = settings.RecentProjects;
        EraseIf(recent,
                [&](const RecentProject& entry)
                {
                    // A directory entry is replaced by its .vproj once known.
                    Path other = Canonical(entry.Location);
                    return other.StrView() == key.StrView()
                        || other.StrView() == key.ParentPath().StrView();
                });
        recent.Insert(recent.begin(), RecentProject{String(name), key,
                                                    static_cast<i64>(std::time(nullptr)),
                                                    String(engineId)});
        if (recent.Size() > 32) recent.Resize(32);
    }

    String ValidateNewProjectDirectory(const Path& directory)
    {
        if (directory.Empty()) return "Choose a location for the project";
        if (!Exists(directory)) return String();
        if (!IsDirectory(directory)) return Format("{} exists and isn't a directory", directory);
        std::error_code ec;
        if (!fs::is_empty(ToFs(directory), ec)) return Format("{} isn't empty", directory);
        return String();
    }
} // namespace Horizon
