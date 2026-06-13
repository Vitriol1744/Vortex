/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/SceneRenderer.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/Entity.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fstream>

struct UniformBufferObject
{
    alignas(16) glm::mat4 Model;
    alignas(16) glm::mat4 ViewProjection;
};

namespace glm
{
    // TODO(v1tr10l7): move somewhere else?
    using json = nlohmann::ordered_json;
    void to_json(json& j, const vec3& vec)
    {
        j = json({{"x", vec.x}, {"y", vec.y}, {"z", vec.z}});
    }
    void from_json(const json& j, vec3& vec)
    {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
    }
    void to_json(json& j, const vec4& vec)
    {
        j = json({{"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w}});
    }
    void from_json(const json& j, vec4& q)
    {
        j.at("x").get_to(q.x);
        j.at("y").get_to(q.y);
        j.at("z").get_to(q.z);
        j.at("w").get_to(q.w);
    }
    void to_json(json& j, const quat& q)
    {
        j = json({{"x", q.x}, {"y", q.y}, {"z", q.z}, {"w", q.w}});
    }
    void from_json(const json& j, quat& vec)
    {
        j.at("x").get_to(vec.x);
        j.at("y").get_to(vec.y);
        j.at("z").get_to(vec.z);
        j.at("w").get_to(vec.w);
    }
}; // namespace glm

namespace Vortex
{
    Entity Scene::AddEntity()
    {
        Entity entity = Entity(m_Registry.create(), *this);

        return entity;
    }

    void Scene::RemoveEntity(Entity entity) { m_Registry.destroy(entity); }

    void Scene::Update(Timestep ts) { IgnoreUnused(ts); }
    void Scene::Render()
    {
        auto view = Registry().view<entt::entity>();
        for (const auto entityID : view)
        {
            Entity entity(entityID, *this);
            if (!entity.HasComponent<MeshComponent>()) continue;

            auto meshComponent = entity.GetComponent<MeshComponent>();

            auto mesh          = meshComponent.Mesh;
            auto material      = meshComponent.Material;
            if (!mesh || !material) continue;

            UniformBufferObject ubo{};
            ubo.Model = glm::mat4(1.0f);
            if (entity.HasComponent<TransformComponent>())
            {
                auto&     tc = entity.GetComponent<TransformComponent>();
                glm::mat4 translationMat
                    = glm::translate(glm::mat4(1.0f), tc.Translation);
                glm::mat4 rotationMat = glm::toMat4(glm::quat(tc.Rotation));
                glm::mat4 scaleMat    = glm::scale(glm::mat4(1.0f), tc.Scale);

                // Combined TRS order (Evaluates Right-to-Left: Scale -> Rotate
                // -> Translate)
                ubo.Model             = translationMat * rotationMat * scaleMat;
            }
            ubo.ViewProjection = MainCamera().ViewProjection();

            auto ub            = meshComponent.UniformBuffer;
            if (ub) ub->SetData(&ubo, sizeof(ubo), 0);

            Renderer::Draw(SceneRenderer::GraphicsPipeline(),
                           mesh->VertexBuffer(), mesh->IndexBuffer(), material);
        }

        Renderer2D::BeginScene(MainCamera());
        auto group
            = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
        for (auto entity : group)
        {
            const auto& [transform, sprite]
                = group.get<TransformComponent, SpriteComponent>(entity);

            Renderer2D::DrawQuad(transform, sprite.Color);
        }

        Renderer2D::EndScene();
    }

    void Scene::Serialize(PathView path)
    {
        using json = nlohmann::ordered_json;
        json out;

        Filesystem::CreateDirectories(path.ParentPath());
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
            if (entity.HasComponent<MeshComponent>())
            {
                auto& mc            = entity.GetComponent<MeshComponent>();
                json  meshComponent = json::object();

                meshComponent["meshPath"]
                    = mc.Mesh ? mc.Mesh->FilePath().Raw() : "";
                meshComponent["texturePath"]
                    = mc.Texture ? mc.Texture->FilePath().Raw() : "";
                // TODO(v1tr10l7): Serialize material references

                components["meshComponent"] = meshComponent;
            }

            entityJ["id"]         = 12345678;
            entityJ["components"] = components;
            entities.emplace_back(entityJ);
        }

        out["entities"] = entities;

        std::ofstream ofs(std::string(path.Raw()));
        ofs << std::setw(4) << out;
    }
    void Scene::Deserialize(PathView path)
    {
        // m_Registry.clear();
        using json = nlohmann::ordered_json;
        std::ifstream ifs(std::string(path.Raw()));
        if (!ifs)
        {
            VtCoreError("Scene::Deserialize: Failed to open '{}'", path);
            return;
        }

        json data = json::parse(ifs);

        if (data.contains("sceneName"))
            m_Name = std::string(data["sceneName"]).data();
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
            auto&       tc   = entity.AddComponent<TagComponent>();
            std::string name = components["tagComponent"]["tag"];
            tc.Name          = String(name.data(), name.size());
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
        if (components.contains("meshComponent"))
        {
            auto&       mc          = entity.AddComponent<MeshComponent>();
            const json& mcJson      = components["meshComponent"];

            PathView    meshPath    = std::string(mcJson["meshPath"]).data();
            PathView    texturePath = std::string(mcJson["texturePath"]).data();

            mc.Mesh = !meshPath.Empty() ? CreateRef<Mesh>(meshPath) : nullptr;
            mc.Texture = !texturePath.Empty() ? Texture2D::Create(texturePath)
                                              : nullptr;
            // TODO(v1tr10l7): Deserialize mesh, material and texture references
        }
    }
} // namespace Vortex
