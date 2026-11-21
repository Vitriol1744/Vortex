/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorCommands.hpp>
#include <Nexus/EditorLayer.hpp>
#include <Nexus/EditorResources.hpp>
#include <Nexus/UI/EditorWindows.hpp>
#include <Nexus/UI/MainMenuBar.hpp>

#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/Image.hpp>
#include <Vortex/Renderer/ImGui/ModalManager.hpp>
#include <Vortex/Renderer/Mesh.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Scene/SceneRenderer.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>
#include <Vortex/Window/Window.hpp>

#include <imgui.h>

#include <cstdlib> // TEMP-AUTOTEST
#include <imgui_internal.h> // TEMP-AUTOTEST

import Vortex.Renderer.Material;

namespace Vortex::Editor
{
    namespace
    {
        constexpr StringView kStartupScene = "assets/scenes/default-scene.vscene";

        Actor AddTexturedMesh(Scene& scene, StringView name, PathView mesh,
                              PathView texture, bool animated)
        {
            auto actor = scene.AddActor();
            actor.AddComponent<TagComponent>(name);
            actor.AddComponent<TransformComponent>();

            auto albedo   = CreateRef<TextureAsset>(texture);
            auto material = Material::Create(SceneRenderer::Shader());
            material->Set("texSampler", albedo->Texture());
            actor.AddComponent<MeshComponent>(CreateRef<Mesh>(mesh), material,
                                              albedo);
            if (animated) actor.AddComponent<AnimationComponent>();
            return actor;
        }

        /// The default scene plus a few test models.
        // TODO(v1tr10l7): drop the test models once projects ship scenes.
        ::Ref<Scene> CreateStartupScene()
        {
            auto scene = CreateRef<Scene>("Vortex-Scene");
            if (!DeserializeJsonFromFile(kStartupScene, *scene))
                VtCoreWarnTag("Editor", "Can't load '{}'", kStartupScene);

            auto room = AddTexturedMesh(*scene, "Viking Room",
                                        "assets/models/viking_room.obj",
                                        "assets/textures/viking_room.png", false);
            room.AddComponent<ScriptComponent>("Gameplay.PlayerController");

            IgnoreUnused(AddTexturedMesh(*scene, "Dancing Vampire",
                                         "assets/models/dancing_vampire.dae",
                                         "assets/textures/vampire.png", true));

            auto trooper = AddTexturedMesh(*scene, "Dancing Stormtrooper",
                                           "assets/models/silly_dancing.fbx",
                                           "assets/textures/Stormtrooper_D.png", true);
            trooper.GetComponent<TransformComponent>().Translation = {2.0f, 0.0f, 0.0f};
            trooper.GetComponent<MeshComponent>().Material->Surface.Roughness = 0.35f;
            return scene;
        }
    } // namespace

    void EditorLayer::OnAttach()
    {
        VtProfileFunction();
        auto& app = *Application::Get();
        Image icon("assets/icons/icon.bmp");
        app.Window().SetIcon(icon);
        app.Window().ShowCursor();

        m_Context = CreateScope<EditorContext>(app);
        m_Toolbar = CreateScope<Toolbar>();
        auto& context = *m_Context;

        RegisterEditorCommands(context);
        RegisterEditorWindows(context);

        Delegate<void(const ProjectChangedEvent&)> projectChanged;
        projectChanged.Bind<&EditorContext::OnProjectChanged>(m_Context.Raw());
        app.SubscribeEvent<ProjectChangedEvent>(projectChanged);

        context.Scenes.SetEditorScene(CreateStartupScene(), kStartupScene);
        // A startup project replaces the scene with its own on the first
        // update. The launcher (Horizon) picks the project on the command
        // line instead:
        //   Nexus --project <file.vproj>
        //   Nexus --new-project <directory> [--name <project name>]
        //         [--template <id>] [--no-gitignore]
        bool        havePreferences = context.LoadPreferences();
        const auto& args            = app.Arguments();
        if (StringView project = args.Value("--project"); !project.Empty())
        {
            auto loaded = EditorProject::Load(project);
            if (!loaded)
                VtCoreErrorTag("Editor", "Can't open the project '{}'",
                               project);
            context.OpenProject(loaded);
        }
        else if (StringView directory = args.Value("--new-project");
                 !directory.Empty())
        {
            ProjectConfig config{};
            StringView    name    = args.Value("--name");
            config.Metadata.Name  = name.Empty()
                                      ? String(Path(directory).BaseName())
                                      : String(name);
            config.StartScenePath = "Assets/Scenes/Main.vscene";

            auto created          = EditorProject::NewFromTemplate(
                config, directory, args.Value("--template"),
                !args.Has("--no-gitignore"));
            if (!created)
                VtCoreErrorTag("Editor", "Can't create a project in '{}'",
                               directory);
            context.OpenProject(created);
        }
        else if (havePreferences && context.Preferences.OpenLastProject)
            context.OpenStartupProject();
    }

    void EditorLayer::OnDetach()
    {
        m_Context->StoreShortcuts();
        m_Context->SavePreferences();
        m_Context->Scenes.Stop();
        m_Toolbar.Reset();
        m_Context.Reset();
        Resources::Clear();
    }

    void EditorLayer::OnUpdate(Timestep deltaTime)
    {
        m_Context->Update(deltaTime);
        // TEMP-AUTOTEST-BEGIN
        static bool autotest = std::getenv("VT_AUTOTEST") != nullptr;
        static int  frame    = 0;
        if (!autotest) return;
        ++frame;
        auto dump = [&](const char* what)
        {
            auto& registry = Scene::Active()->Registry();
            auto  view     = registry.view<TagComponent, TransformComponent>();
            int   shown    = 0;
            for (auto e : view)
            {
                auto& tag = view.get<TagComponent>(e);
                if (tag.Name.View() != "Ball" && tag.Name.View() != "Player" && !(tag.Name.View() == "Crate" && shown < 3))
                    continue;
                auto& t = view.get<TransformComponent>(e).Translation;
                VtCoreInfoTag("AUTOTEST", "{} f{} state={} {} #{} pos=({:.2f},{:.2f},{:.2f}) bodies={}",
                              what, frame, int(m_Context->Scenes.State()), tag.Name.View(),
                              u32(e), t.x, t.y, t.z,
                              Scene::Active()->Physics() ? 1 : 0);
                ++shown;
            }
        };
        auto run = [&](StringView command)
        {
            VtCoreInfoTag("AUTOTEST", "f{} run {}", frame, command);
            m_Context->Commands.Execute(command);
        };
        if (StringView(std::getenv("VT_AUTOTEST")) == "inspect")
        {
            auto& registry = Scene::Active()->Registry();
            if (frame == 40)
            {
                Actor actor = Scene::Active()->AddActor();
                actor.AddComponent<TagComponent>("Script Test");
                actor.AddComponent<TransformComponent>();
                auto& script = actor.AddComponent<ScriptComponent>("Gameplay.PlayerController");
                ScriptFieldValue speed;
                speed.Name = "MoveSpeed";
                speed.Type = ScriptFieldType::eFloat;
                speed.Set(7.5f);
                script.Fields.PushBack(speed);
                m_Context->Select(actor);
                VtCoreInfoTag("AUTOTEST", "inspect step 1 (script)");
            }
            if (frame > 270 && frame < 470)
                if (auto* window = ImGui::FindWindowByName("Inspector"))
                    ImGui::SetScrollY(window, 330.0f);
            if (frame == 260)
                for (auto e : registry.view<MeshComponent>())
                {
                    if (!registry.get<MeshComponent>(e).Texture) continue;
                    m_Context->Select(e);
                    VtCoreInfoTag("AUTOTEST", "inspect step 2 (mesh)");
                    break;
                }
            if (frame == 480)
            {
                m_Context->ShowSettings = true;
                VtCoreInfoTag("AUTOTEST", "inspect step 3 (settings)");
            }
            return;
        }
        if (StringView(std::getenv("VT_AUTOTEST")).StartsWith("fps"))
        {
            if (frame == 30) run("Scene.NewFpsTest");
            if (frame == 90 && StringView(std::getenv("VT_AUTOTEST")) != "fps-edit") run("Scene.Play");
            if (frame % 40 == 0)
            {
                auto& registry = Scene::Active()->Registry();
                for (auto e : registry.view<TagComponent, TransformComponent>())
                {
                    auto& tag = registry.get<TagComponent>(e).Name;
                    if (tag.View() != "Player" && tag.View() != "Crate" && tag.View() != "Ragdoll Chest")
                        continue;
                    auto& t = registry.get<TransformComponent>(e).Translation;
                    VtCoreInfoTag("AUTOTEST", "f{} {} ({:.2f},{:.2f},{:.2f}) fps {:.1f}", frame,
                                  tag.View(), t.x, t.y, t.z, ImGui::GetIO().Framerate);
                    if (tag.View() == "Crate") break;
                }
            }
            return;
        }
        switch (frame)
        {
            case 30: run("Scene.NewPhysicsTest"); break;
            case 160:
                if (StringView(std::getenv("VT_AUTOTEST")) == "exit-sim")
                {
                    VtCoreInfoTag("AUTOTEST", "f{} closing while simulating", frame);
                    Application::Get()->Close();
                }
                break;
            case 100:
                if (StringView(std::getenv("VT_AUTOTEST")) == "exit-edit")
                {
                    VtCoreInfoTag("AUTOTEST", "f{} closing while editing", frame);
                    Application::Get()->Close();
                }
                break;
            case 120: dump("edit"); run("Scene.Simulate"); break;
            case 200: dump("sim"); break;
            case 400: dump("sim"); run("Scene.Stop"); break;
            case 410: dump("stopped"); run("Scene.Play"); break;
            case 500: dump("play"); break;
            case 650: dump("play"); run("Scene.Stop"); break;
            case 660: dump("stopped2"); break;
            default:
                if (frame > 410 && frame < 650 && frame % 30 == 0) dump("playtrace");
                break;
        }
        // TEMP-AUTOTEST-END
    }

    void EditorLayer::OnRender() { m_Context->Windows.Render(*m_Context); }

    void EditorLayer::OnImGuiRender()
    {
        VtProfileFunction();
        auto& context = *m_Context;
        context.Commands.ProcessShortcuts();

        DrawDockspace();
        context.Windows.Draw(context);
        UI::ModalManager::RenderModals();
        m_SettingsOverlay.Render(context);
    }

    void EditorLayer::DrawDockspace()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        constexpr ImGuiWindowFlags flags
            = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        // Names kept from older versions: imgui.ini keys the saved layout by
        // them.
        ImGui::Begin("Dockspace Demo", nullptr, flags);
        ImGui::PopStyleVar(3);

        DrawMainMenuBar(*m_Context);
        m_Toolbar->Draw(*m_Context);
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
            ImGui::DockSpace(ImGui::GetID("MyDockSpace"));

        ImGui::End();
    }
} // namespace Vortex::Editor
