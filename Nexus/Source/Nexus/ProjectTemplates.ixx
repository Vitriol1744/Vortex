/*
 * Created by v1tr10l7 on 21.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

export module Nexus.ProjectTemplates;

import Prism.Algorithm.Sort;
import Prism.Containers.Vector;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Utility.Optional;
import Prism.Utility.Path;
import Prism.Utility.PathView;

/*
 * Project templates ship with the engine, one directory each:
 *
 *   assets/templates/<id>/template.json
 *     {
 *       "name": "Physics Playground",
 *       "description": "...",
 *       "category": "3D",
 *       "icon": "bolt",
 *       "order": 20,
 *       "startScene": "Assets/Scenes/Main.vscene",
 *       "generator": "physics"
 *     }
 *   assets/templates/<id>/content/...   copied into the new project as is
 *
 * `generator` (optional) names a scene Nexus builds in code and saves as the
 * start scene: "empty" or "physics". See assets/templates/README.md.
 */
export namespace Vortex::Editor
{
    using Prism::Optional;
    using Prism::Path;
    using Prism::PathView;
    using Prism::String;
    using Prism::StringView;
    using Prism::Vector;

    struct ProjectTemplate
    {
        String Id;
        String Name;
        String Description;
        String Category;
        String Icon;
        int    Order = 100;
        /// Relative to the project root.
        Path   StartScene = "Assets/Scenes/Main.vscene";
        String Generator;
        /// The template's directory (holds template.json and content/).
        Path   Directory;

        Path   ContentDirectory() const { return Directory / "content"; }
    };

    /// Where the engine's templates are, relative to the working directory.
    constexpr const char* kTemplateDirectory = "assets/templates";
    /// Used when nothing else is chosen.
    constexpr const char* kDefaultTemplate   = "starter-3d";

    /// Every valid template, in display order.
    Vector<ProjectTemplate> ListProjectTemplates();
    Optional<ProjectTemplate> FindProjectTemplate(StringView id);
} // namespace Vortex::Editor

namespace Vortex::Editor
{
    namespace
    {
        namespace fs = std::filesystem;

        String     ToString(const std::string& text) { return String(StringView(text.data(), text.size())); }
        String     GetString(const nlohmann::json& object, const char* key, StringView fallback = {})
        {
            if (!object.contains(key) || !object[key].is_string()) return String(fallback);
            return ToString(object[key].get<std::string>());
        }

        Optional<ProjectTemplate> ReadTemplate(const fs::path& directory)
        {
            std::ifstream file(directory / "template.json");
            if (!file) return Prism::NullOpt;
            auto root = nlohmann::json::parse(file, nullptr, false);
            if (root.is_discarded() || !root.is_object()) return Prism::NullOpt;

            ProjectTemplate result;
            result.Id          = ToString(directory.filename().string());
            result.Name        = GetString(root, "name", result.Id);
            result.Description = GetString(root, "description");
            result.Category    = GetString(root, "category");
            result.Icon        = GetString(root, "icon", "cube");
            result.Order       = root.value("order", 100);
            result.StartScene  = Path(GetString(root, "startScene", "Assets/Scenes/Main.vscene"));
            result.Generator   = GetString(root, "generator");
            result.Directory   = Path(ToString(directory.string()));
            return result;
        }
    } // namespace

    Vector<ProjectTemplate> ListProjectTemplates()
    {
        Vector<ProjectTemplate> templates;
        std::error_code         ec;
        for (const auto& entry : fs::directory_iterator(kTemplateDirectory, ec))
            if (entry.is_directory())
                if (auto found = ReadTemplate(entry.path())) templates.PushBack(*found);
        Prism::Sort(templates.begin(), templates.end(),
                    [](const ProjectTemplate& a, const ProjectTemplate& b)
                    { return a.Order != b.Order ? a.Order < b.Order : a.Name < b.Name; });
        return templates;
    }

    Optional<ProjectTemplate> FindProjectTemplate(StringView id)
    {
        // An id is a directory name, never a path.
        if (id.Empty() || id.Find('/') != StringView::NPos || id.Find('\\') != StringView::NPos
            || id == "." || id == "..")
            return Prism::NullOpt;
        return ReadTemplate(fs::path(kTemplateDirectory) / std::string(id.Raw(), id.Size()));
    }
} // namespace Vortex::Editor
