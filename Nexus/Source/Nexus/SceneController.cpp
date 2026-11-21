/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorContext.hpp>
#include <Nexus/SceneController.hpp>

#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>
#include <Vortex/Window/Window.hpp>

import Prism.Containers.Array;

namespace Vortex::Editor
{
    namespace
    {
        auto s_SceneFilter
            = ToArray<Window::FileFilter>({{"Vortex Scene file", "vscene"}});

        template <typename... Components>
        void CopyComponents(entt::registry& destination, const entt::registry& source)
        {
            (
                [&]
                {
                    auto view = source.view<const Components>();
                    for (auto entity : view)
                        destination.emplace_or_replace<Components>(
                            entity, view.template get<const Components>(entity));
                }(),
                ...);
        }

        /// A runtime copy sharing the assets (meshes, materials...) but none of
        /// the per-entity state. Entity ids are kept, so the selection and
        /// script instances map between both.
        ::Ref<Scene> CopyScene(const Scene& source)
        {
            VtProfileFunction();
            auto copy = CreateRef<Scene>(source.Name());
            auto& destination = copy->Registry();
            for (auto entity : source.Registry().view<entt::entity>())
            {
                [[maybe_unused]] auto created = destination.create(entity);
                VtCoreAssert(created == entity);
            }
            CopyComponents<TagComponent, TransformComponent, CameraComponent,
                           SpriteComponent, MeshComponent, AnimationComponent,
                           ScriptComponent, RigidBodyComponent, BoxColliderComponent,
                           SphereColliderComponent, CapsuleColliderComponent,
                           MeshColliderComponent, JointComponent,
                           FirstPersonControllerComponent, WeaponComponent,
                           WeaponPartComponent, BulletDecalComponent>(destination,
                                                                      source.Registry());
            return copy;
        }
    } // namespace

    SceneController::SceneController(EditorContext& context)
        : m_Context(context)
    {
    }

    void SceneController::Activate(::Ref<Scene> scene)
    {
        scene->SetMainCamera(m_Context.Camera);
        Scene::SwitchTo(scene);
        // Same entity ids in the edit scene and its runtime copy.
        m_Context.RetargetSelection();
    }

    void SceneController::SetEditorScene(::Ref<Scene> scene, PathView path)
    {
        if (IsRunning()) Stop();
        m_EditorScene = scene;
        m_ScenePath   = path;
        m_Context.ClearSelection();
        Activate(scene);
    }

    void SceneController::NewScene()
    {
        SetEditorScene(CreateRef<Scene>("Untitled"));
    }

    void SceneController::OpenScene(PathView path) { m_PendingPath = path; }

    void SceneController::OpenSceneDialog()
    {
        if (auto path = Window::OpenFile(s_SceneFilter, "."_pv)) OpenScene(*path);
    }

    bool SceneController::Load(PathView path)
    {
        auto scene = CreateRef<Scene>();
        if (!DeserializeJsonFromFile(path, *scene))
        {
            VtCoreErrorTag("Editor", "Can't load the scene '{}'", path);
            return false;
        }
        SetEditorScene(scene, path);
        VtCoreInfoTag("Editor", "Opened the scene '{}'", path);
        return true;
    }

    bool SceneController::SaveScene()
    {
        if (m_ScenePath.Empty()) return SaveSceneAs();
        if (!SerializeJsonToFile(m_ScenePath, *m_EditorScene))
        {
            VtCoreErrorTag("Editor", "Can't save the scene to '{}'", m_ScenePath);
            return false;
        }
        VtCoreInfoTag("Editor", "Saved the scene to '{}'", m_ScenePath);
        return true;
    }

    bool SceneController::SaveSceneAs()
    {
        auto path = Window::SaveFile(s_SceneFilter);
        if (!path) return false;
        m_ScenePath = *path;
        return SaveScene();
    }

    void SceneController::ReloadScene()
    {
        if (m_ScenePath.Empty())
        {
            OpenSceneDialog();
            return;
        }
        OpenScene(m_ScenePath);
    }

    void SceneController::Start(SceneState state)
    {
        if (IsRunning()) Stop();
        m_State        = state;
        m_Paused       = false;
        m_StepFrames   = 0;
        m_RuntimeScene = CopyScene(*m_EditorScene);
        Activate(m_RuntimeScene);
        if (state == SceneState::ePlay) m_RuntimeScene->OnRuntimeStart();
        else m_RuntimeScene->OnSimulationStart();
    }

    void SceneController::Play() { Start(SceneState::ePlay); }
    void SceneController::Simulate() { Start(SceneState::eSimulate); }

    void SceneController::Stop()
    {
        if (!IsRunning()) return;
        m_RuntimeScene->OnRuntimeStop();
        m_State        = SceneState::eEdit;
        m_Paused       = false;
        m_StepFrames   = 0;
        m_RuntimeScene = nullptr;
        Activate(m_EditorScene);
    }

    void SceneController::SetPaused(bool paused)
    {
        if (IsRunning()) m_Paused = paused;
    }

    void SceneController::Step(u32 frames)
    {
        if (IsRunning() && m_Paused) m_StepFrames += frames;
    }

    void SceneController::Update(Timestep deltaTime)
    {
        VtProfileFunction();
        if (!m_PendingPath.Empty())
        {
            Path path = m_PendingPath;
            m_PendingPath = "";
            Load(path);
        }
        if (!m_EditorScene) return;

        Scene& scene = *Scene::Active();
        switch (m_State)
        {
            // Animations keep previewing while editing; scripts only run once
            // a runtime start created their instances.
            case SceneState::eEdit: scene.Update(deltaTime); break;
            case SceneState::ePlay:
            case SceneState::eSimulate:
            {
                Timestep step = deltaTime;
                if (m_Paused)
                {
                    if (m_StepFrames == 0) break;
                    --m_StepFrames;
                    step = Timestep(1.0f / 60.0f);
                }
                if (m_State == SceneState::ePlay) scene.UpdateRuntime(step);
                else scene.UpdateSimulation(step);
                break;
            }
        }
    }
} // namespace Vortex::Editor
