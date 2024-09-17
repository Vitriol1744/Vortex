/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/Renderer2D.hpp"
#include "Vortex/Scene/Entity.hpp"
#include "Vortex/Scene/Scene.hpp"

#include <nlohmann/json.hpp>

namespace Vortex
{
    Entity Scene::AddEntity()
    {
        Entity entity = Entity(m_Registry.create(), *this);

        return entity;
    }

    void Scene::DrawEntities()
    {
        auto group
            = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
        for (auto entity : group)
        {
            const auto& [transform, sprite]
                = group.get<TransformComponent, SpriteComponent>(entity);

            Renderer2D::DrawQuad(transform, sprite.Color);
        }
    }

    void Scene::Serialize()
    {
        using json = nlohmann::ordered_json;
        json out;

        std::filesystem::create_directories("assets/scenes");

        std::string sceneName = "Unnamed";
        out["sceneName"]      = sceneName;

        json entities         = json::array();
        for (const auto entityID : m_Registry.view<entt::entity>())
        {
            Entity entity(entityID, *this);
            json   entityJ    = json::object();
            json   components = json::object();

            if (entity.HasComponent<TagComponent>())
            {
                auto& tc            = entity.GetComponent<TagComponent>();
                json  tagComponent  = json::object();
                tagComponent["tag"] = tc.Name;
                components["TagComponent"] = tagComponent;
            }

            entityJ["ID"]         = 12345678;
            entityJ["Components"] = components;
            entities.emplace_back(entityJ);
        }

        out["entities"] = entities;

        std::transform(sceneName.begin(), sceneName.end(), sceneName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        std::filesystem::path path
            = fmt::format("assets/scenes/{}.vproj", sceneName);
        std::ofstream ofs(path);
        ofs << std::setw(4) << out;
    }
    void Deserialize();
} // namespace Vortex
