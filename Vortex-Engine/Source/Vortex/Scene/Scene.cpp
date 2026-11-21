/*
 * Created by v1tr10l7 on 11.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Filesystem/Filesystem.hpp>
#include <Vortex/Scene/Actor.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/FirstPerson.hpp>
#include <Vortex/Scene/Scene.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

import Vortex.Physics;


namespace Vortex
{
    Actor Scene::AddActor()
    {
        Actor actor = Actor(m_Registry.create(), *this);

        return actor;
    }

    void Scene::RemoveActor(Actor actor) { m_Registry.destroy(actor); }

    Scene::~Scene()
    {
        if (m_ScriptsRunning) ScriptEngine::ClearScriptInstances();
        StopPhysics();
    }

    void Scene::StartPhysics()
    {
        StopPhysics();
        if (!PhysicsEngine::IsInitialized()) return;
        m_Physics = new PhysicsScene(*this);
    }

    void Scene::StopPhysics()
    {
        delete m_Physics;
        m_Physics = nullptr;
    }

    void Scene::OnSimulationStart() { StartPhysics(); }

    void Scene::OnRuntimeStop()
    {
        FirstPerson::Stop(*this);
        if (m_ScriptsRunning) ScriptEngine::ClearScriptInstances();
        m_ScriptsRunning = false;
        StopPhysics();
    }

    void Scene::OnRuntimeStart()
    {
        // Bodies exist before scripts' OnCreate, so they can push them.
        StartPhysics();
        m_ScriptsRunning = true;

        auto view = m_Registry.view<ScriptComponent>();

        VtCoreTrace("OnRuntimeStart");
        for (auto entityID : view)
        {
            Actor actor{entityID, *this};
            auto& sc = actor.GetComponent<ScriptComponent>();

            if (!sc.ClassName.Empty())
            {
                VtCoreTrace("Creating instance of script `{}`", sc.ClassName);
                ScriptEngine::OnCreateEntity((u64)entityID, sc);
            }
        }
    }

    void Scene::UpdateAnimations(Timestep ts)
    {
        VtProfileFunction();
        auto view = m_Registry.view<AnimationComponent, MeshComponent>();
        for (auto entity : view)
        {
            auto& animation = view.get<AnimationComponent>(entity);
            auto& mesh      = view.get<MeshComponent>(entity).Mesh;
            if (!mesh || !mesh->IsSkinned())
            {
                animation.Pose.Clear();
                continue;
            }

            const auto& clips = mesh->Animations();
            if (clips.Empty() || animation.Clip >= clips.Size())
            {
                Animation::BindPose(mesh->Skeleton(), animation.Pose);
                continue;
            }
            const auto& clip = clips[animation.Clip];
            if (animation.Playing)
            {
                animation.Time += static_cast<f32>(ts.Seconds()) * animation.Speed;
                f32 duration = clip.DurationSeconds();
                if (duration > 0.0f && animation.Time > duration)
                    animation.Time -= duration * static_cast<i32>(animation.Time / duration);
            }
            Animation::Evaluate(mesh->Skeleton(), clip, animation.Time, animation.Pose);
        }
    }

    void Scene::Update(Timestep ts) { UpdateAnimations(ts); }

    void Scene::UpdateRuntime(Timestep ts)
    {
        VtProfileFunction();
        f32 deltaTime = static_cast<f32>(ts.Seconds());
        if (m_ScriptsRunning) ScriptEngine::OnUpdateRuntime(ts);
        FirstPerson::Update(*this, deltaTime);
        if (m_Physics) m_Physics->Simulate(deltaTime);
        // After physics, so the camera and gun follow where the body ended.
        FirstPerson::LateUpdate(*this, deltaTime);
        UpdateAnimations(ts);
    }

    void Scene::UpdateSimulation(Timestep ts)
    {
        VtProfileFunction();
        if (m_Physics) m_Physics->Simulate(static_cast<f32>(ts.Seconds()));
        UpdateAnimations(ts);
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
            if (actor.HasComponent<AnimationComponent>())
                SerializeValue(archive, "animationComponent",
                               actor.GetComponent<AnimationComponent>());
            if (actor.HasComponent<ScriptComponent>())
                SerializeValue(archive, "scriptComponent",
                               actor.GetComponent<ScriptComponent>());
            if (actor.HasComponent<RigidBodyComponent>())
                SerializeValue(archive, "rigidBodyComponent",
                               actor.GetComponent<RigidBodyComponent>());
            if (actor.HasComponent<BoxColliderComponent>())
                SerializeValue(archive, "boxColliderComponent",
                               actor.GetComponent<BoxColliderComponent>());
            if (actor.HasComponent<SphereColliderComponent>())
                SerializeValue(archive, "sphereColliderComponent",
                               actor.GetComponent<SphereColliderComponent>());
            if (actor.HasComponent<CapsuleColliderComponent>())
                SerializeValue(archive, "capsuleColliderComponent",
                               actor.GetComponent<CapsuleColliderComponent>());
            if (actor.HasComponent<MeshColliderComponent>())
                SerializeValue(archive, "meshColliderComponent",
                               actor.GetComponent<MeshColliderComponent>());

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

                AnimationComponent animation;
                if (DeserializeValue(reader, "animationComponent", animation))
                    actor.AddComponent<AnimationComponent>(animation);

                ScriptComponent script;
                if (DeserializeValue(reader, "scriptComponent", script))
                    actor.AddComponent<ScriptComponent>(script);

                RigidBodyComponent rigidBody;
                if (DeserializeValue(reader, "rigidBodyComponent", rigidBody))
                    actor.AddComponent<RigidBodyComponent>(rigidBody);

                BoxColliderComponent box;
                if (DeserializeValue(reader, "boxColliderComponent", box))
                    actor.AddComponent<BoxColliderComponent>(box);

                SphereColliderComponent sphere;
                if (DeserializeValue(reader, "sphereColliderComponent", sphere))
                    actor.AddComponent<SphereColliderComponent>(sphere);

                CapsuleColliderComponent capsule;
                if (DeserializeValue(reader, "capsuleColliderComponent", capsule))
                    actor.AddComponent<CapsuleColliderComponent>(capsule);

                MeshColliderComponent meshCollider;
                if (DeserializeValue(reader, "meshColliderComponent", meshCollider))
                    actor.AddComponent<MeshColliderComponent>(meshCollider);

                reader.EndObject(); // "components"
            }

            reader.EndObject(); // Entity object
        }

        reader.EndArray(); // "entities"
        return true;
    }
} // namespace Vortex
