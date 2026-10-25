/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/SceneRenderer.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

struct UniformBufferObject
{
    alignas(16) glm::mat4 Model;
    alignas(16) glm::mat4 ViewProjection;
};

namespace Vortex
{
    Actor Scene::AddActor()
    {
        Actor actor = Actor(m_Registry.create(), *this);

        return actor;
    }

    void Scene::RemoveActor(Actor actor) { m_Registry.destroy(actor); }

    void Scene::OnRuntimeStart()
    {
        auto view = m_Registry.view<ScriptComponent>();

        VtCoreTrace("OnRuntimeStart");
        for (auto entityID : view)
        {
            Actor actor{entityID, *this};
            auto& sc = actor.GetComponent<ScriptComponent>();

            if (!sc.ClassName.Empty())
            {
                VtCoreTrace("Creating instance of script `{}`", sc.ClassName);
                ScriptEngine::OnCreateEntity((u64)entityID, sc.ClassName);
            }
        }
    }

    void Scene::Update(Timestep ts) 
    { 
        ScriptEngine::OnUpdateRuntime(ts);
        return;

        auto view = Registry().view<ScriptComponent>();
        for (const auto entity : view)
        {
            Actor actor{entity, *this};
            auto& sc = actor.GetComponent<ScriptComponent>();

            if (!sc.ClassName.Empty())
                ScriptEngine::OnUpdateEntity((u64)entity, ts);
        }
        IgnoreUnused(ts); 
    }
    void Scene::Render()
    {
        auto view = Registry().view<entt::entity>();
        for (const auto entity : view)
        {
            Actor actor(entity, *this);
            if (!actor.HasComponent<MeshComponent>()) continue;

            auto meshComponent = actor.GetComponent<MeshComponent>();

            auto mesh          = meshComponent.Mesh;
            auto material      = meshComponent.Material;
            if (!mesh || !material) continue;

            UniformBufferObject ubo{};
            ubo.Model = glm::mat4(1.0f);
            if (actor.HasComponent<TransformComponent>())
            {
                auto&     tc = actor.GetComponent<TransformComponent>();
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

        Renderer2D::BeginScene(MainCamera().ViewProjection());
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

    void Scene::Serialize(ArchiveWriter& archive) const
    {
        archive.Write("sceneName", m_Name.View());

        auto entityView = m_Registry.view<entt::entity>();
        archive.BeginArray("entities", 0);
        for (const auto entity : entityView)
        {
            Actor actor(entity, const_cast<Scene&>(*this));
            archive.BeginObject("");
            archive.Write("id", 12345678);

            archive.BeginObject("components");
            if (actor.HasComponent<TagComponent>())
                SerializeValue(archive, "tagComponent",
                               actor.GetComponent<TagComponent>());
            if (actor.HasComponent<TransformComponent>())
                SerializeValue(archive, "transformComponent",
                               actor.GetComponent<TransformComponent>());
            if (actor.HasComponent<SpriteComponent>())
                SerializeValue(archive, "spriteComponent",
                               actor.GetComponent<SpriteComponent>());
            if (actor.HasComponent<MeshComponent>())
                SerializeValue(archive, "meshComponent",
                               actor.GetComponent<MeshComponent>());

            archive.EndObject();
            archive.EndObject();
        }
        archive.EndArray();
    }
    bool Scene::Deserialize(ArchiveReader& reader)
    {
        m_Registry.clear();
        reader.Read("sceneName", m_Name);
        VtCoreTrace("scene Name => {}", m_Name);

        usize entityCount = 0;
        if (!reader.BeginArray("entities", entityCount)) return false;
        VtCoreTrace("Deserializing {} entities", entityCount);

        for (usize i = 0; i < entityCount; ++i)
        {
            if (!reader.BeginObject("")) continue;

            Actor actor = AddActor();

            u64   id    = 0;
            reader.Read("id", id);

            if (reader.BeginObject("components"))
            {
                TagComponent tag;
                if (DeserializeValue(reader, "tagComponent", tag))
                {
                    VtCoreInfo("Tag: {}", tag.Name);
                    actor.AddComponent<TagComponent>(tag);
                }

                TransformComponent transform;
                if (DeserializeValue(reader, "transformComponent", transform))
                {
                    actor.AddComponent<TransformComponent>(transform);
                }

                SpriteComponent sprite;
                if (DeserializeValue(reader, "spriteComponent", sprite))
                    actor.AddComponent<SpriteComponent>(sprite);

                MeshComponent mesh;
                if (DeserializeValue(reader, "meshComponent", mesh))
                    actor.AddComponent<MeshComponent>(mesh);

                reader.EndObject(); // "components"
            }

            reader.EndObject(); // Entity object
        }

        reader.EndArray(); // "entities"
        return true;
    }
} // namespace Vortex
