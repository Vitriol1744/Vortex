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

export module Horizon.Templates;

import Prism.Algorithm.Sort;
import Horizon.Installs;
import Horizon.Json;
import Horizon.Platform;

/*
 * The project templates an engine version ships (assets/templates/<id>/
 * template.json, see that directory's README). Horizon only lists them and
 * passes the chosen id to Nexus (`--template <id>`), which creates the
 * project; older engines without templates create their default project.
 */
export namespace Horizon
{
    struct ProjectTemplate
    {
        String Id;
        String Name;
        String Description;
        String Category;
        String Icon;
        int    Order = 100;
    };

    /// The id Nexus uses when none is given.
    constexpr const char*   kDefaultTemplate = "starter-3d";

    /// `engine`'s templates in display order; empty if it has none.
    Vector<ProjectTemplate> ScanTemplates(const EngineInstall& engine)
    {
        Vector<ProjectTemplate> templates;
        Path                    root = engine.WorkingDirectory / "assets" / "templates";
        std::error_code         ec;
        for (const auto& entry : fs::directory_iterator(ToFs(root), ec))
        {
            if (!entry.is_directory()) continue;
            Path        directory = FromFs(entry.path());
            Json::Value manifest  = Json::ReadFile(directory / "template.json");
            if (manifest.is_discarded() || !manifest.is_object()) continue;

            ProjectTemplate entryTemplate;
            entryTemplate.Id          = String(directory.BaseName());
            entryTemplate.Name        = Json::GetString(manifest, "name", entryTemplate.Id);
            entryTemplate.Description = Json::GetString(manifest, "description");
            entryTemplate.Category    = Json::GetString(manifest, "category");
            entryTemplate.Icon        = Json::GetString(manifest, "icon", "cube");
            entryTemplate.Order       = Json::Get(manifest, "order", 100);
            templates.PushBack(Move(entryTemplate));
        }
        Prism::Sort(templates.begin(), templates.end(),
                    [](const ProjectTemplate& a, const ProjectTemplate& b)
                    { return a.Order != b.Order ? a.Order < b.Order : a.Name < b.Name; });
        return templates;
    }
} // namespace Horizon
