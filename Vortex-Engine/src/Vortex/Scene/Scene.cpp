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

namespace glm
{
    // TODO(v1tr10l7): move somewhere else?
    using json = nlohmann::ordered_json;
    void to_json(json& j, const glm::vec3& vec)
    {
        j = json({{"x", vec.x}, {"y", vec.y}, {"z", vec.z}});
    }
    void from_json(const json& j, glm::vec3& vec)
    {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
    }
    void to_json(json& j, const glm::vec4& vec)
    {
        j = json({{"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w}});
    }
    void from_json(const json& j, glm::vec4& q)
    {
        j.at("x").get_to(q.x);
        j.at("y").get_to(q.y);
        j.at("z").get_to(q.z);
        j.at("w").get_to(q.w);
    }
    void to_json(json& j, const glm::quat& q)
    {
        j = json({{"x", q.x}, {"y", q.y}, {"z", q.z}, {"w", q.w}});
    }
    void from_json(const json& j, glm::quat& vec)
    {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
        j.at("w").get_to(vec.w);
    }
} // namespace glm

namespace Vortex
{
    Entity Scene::AddEntity()
    {
        Entity entity = Entity(m_Registry.create(), *this);

        return entity;
    }

    void Scene::RemoveEntity(Entity entity) { m_Registry.destroy(entity); }

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

    void Scene::Serialize(PathView path)
    {
        using json = nlohmann::ordered_json;
        json out;

        std::filesystem::create_directories(path.parent_path());
        VtCoreTrace("Saving scene at '{}'", path);

        out["sceneName"] = m_Name;

        json entities    = json::array();
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
                components["tagComponent"] = tagComponent;
            }
            if (entity.HasComponent<TransformComponent>())
            {
                auto& tc = entity.GetComponent<TransformComponent>();
                json  transformComponent          = json::object();

                transformComponent["translation"] = tc.Translation;
                transformComponent["rotation"]    = tc.Rotation;
                transformComponent["scale"]       = tc.Scale;
                components["transformComponent"]  = transformComponent;
            }
            if (entity.HasComponent<SpriteComponent>())
            {
                auto& sc              = entity.GetComponent<SpriteComponent>();
                json  spriteComponent = json::object();
                spriteComponent["color"]      = sc.Color;

                components["spriteComponent"] = spriteComponent;
            }

            entityJ["id"]         = 12345678;
            entityJ["components"] = components;
            entities.emplace_back(entityJ);
        }

        out["entities"] = entities;

        std::ofstream ofs(path);
        ofs << std::setw(4) << out;
    }
    void Scene::Deserialize(PathView path)
    {
        m_Registry.clear();
        using json = nlohmann::ordered_json;
        std::ifstream ifs(path);
        if (!ifs)
        {
            VtCoreError("Scene::Deserialize: Failed to open '{}'", path);
            return;
        }

        json data = json::parse(ifs);

        if (data.contains("sceneName")) m_Name = data["sceneName"];
        if (data.contains("entities")) DeserializeEntities(data["entities"]);
    }

    void Scene::DeserializeEntities(const nlohmann::ordered_json& entities)
    {
        using json = nlohmann::ordered_json;
        for (const json& entityJson : entities)
        {
            Entity entity = AddEntity();
            if (entityJson.contains("components"))
                DeserializeComponents(entityJson["components"], entity);
        }
    }
    void Scene::DeserializeComponents(const nlohmann::ordered_json& components,
                                      Entity&                       entity)
    {
        using json = nlohmann::ordered_json;
        if (components.contains("tagComponent"))
        {
            auto& tc = entity.AddComponent<TagComponent>();
            tc.Name  = components["tagComponent"]["tag"];
        }
        if (components.contains("transformComponent"))
        {
            auto&       tc     = entity.AddComponent<TransformComponent>();
            const json& tcJson = components["transformComponent"];
            tc.Translation     = tcJson["translation"];
            tc.Rotation        = tcJson["rotation"];
            tc.Scale           = tcJson["scale"];
        }
        if (components.contains("spriteComponent"))
        {
            auto& sc = entity.AddComponent<SpriteComponent>();
            sc.Color = components["spriteComponent"]["color"];
        }
    }
} // namespace Vortex
