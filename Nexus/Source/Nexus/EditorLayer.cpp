/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Nexus/EditorLayer.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <Vortex/Core/Math/Ray.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Engine/Application.hpp>

#include <Vortex/Renderer/ImGui/ImGui.hpp>
#include <Vortex/Renderer/Image.hpp>
#include <Vortex/Renderer/RenderTargetWindow.hpp>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/SceneRenderer.hpp>
#include <Vortex/Renderer/TextureAsset.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Script/ScriptEngine.hpp>
#include <Vortex/Serialization/JsonArchive.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>

using namespace Vortex::Math;

namespace Vortex::Editor
{
    // TODO(v1tr10l7): Shouldn't be defined here
    using PipelineSpecification = GraphicsPipelineSpecification;
    using Pipeline              = GraphicsPipeline;
    static u32   frameIndex     = 0;

    static Array s_ProjectFileFilter
        = ToArray<Window::FileFilter>({{"Vortex Project file", "vproj"}});
    static Array s_SceneFileFilter
        = ToArray<Window::FileFilter>({{"Vortex Scene file", "vscene"}});

    void EditorLayer::OnAttach()
    {
        VtProfileFunction();
        auto& window = m_App->Window();
        // TODO(v1tr10l7): Should be handled by the asset manager or something
        Image image("assets/icons/icon.bmp");

        window.SetIcon(image);
        window.ShowCursor();

        m_Shader = SceneRenderer::Shader();

        // TODO(v1tr10l7): this is just for testing,
        auto vikingRoomTexture
            = CreateRef<TextureAsset>("assets/textures/viking_room.png");
        auto vampireTexture
            = CreateRef<TextureAsset>("assets/textures/vampire.png");
        auto vikingRoomMesh = CreateRef<Mesh>("assets/models/viking_room.obj");

        if (!Scene::Active()) Scene::SwitchTo(CreateRef<Scene>());

        m_CurrentScenePath = "assets/scenes/default-scene.vscene";
        auto& scene        = *Scene::Active();
        scene.SetName("Vortex-Scene");
        DeserializeJsonFromFile(m_CurrentScenePath, scene);
        scene.SetMainCamera(m_Camera);

        auto vikingRoomActor = scene.AddActor();
        vikingRoomActor.AddComponent<TagComponent>("Viking Room");
        vikingRoomActor.AddComponent<ScriptComponent>(
            "Gameplay.PlayerController");
        vikingRoomActor.AddComponent<TransformComponent>();

        auto vikingRoomMaterial = Material::Create(m_Shader);
        auto vikingRoomUniformBuffer
            = UniformBuffer::Create(sizeof(UniformBufferObject));
        vikingRoomMaterial->Set("UniformBufferObject", vikingRoomUniformBuffer);
        vikingRoomMaterial->Set("texSampler", vikingRoomTexture->Texture());
        auto& vrmc = vikingRoomActor.AddComponent<MeshComponent>(
            vikingRoomMesh, vikingRoomMaterial, vikingRoomTexture);
        vrmc.UniformBuffer = vikingRoomUniformBuffer;
        auto vampireActor  = scene.AddActor();
        vampireActor.AddComponent<TagComponent>("Dancing Vampire");

        auto vampireMesh = CreateRef<Mesh>("assets/models/dancing_vampire.dae");
        auto vampireMaterial = Material::Create(m_Shader);
        auto vampireUniformBuffer
            = UniformBuffer::Create(sizeof(UniformBufferObject));
        vampireMaterial->Set("UniformBufferObject", vampireUniformBuffer);
        vampireMaterial->Set("texSampler", vampireTexture->Texture());
        auto& vmc = vampireActor.AddComponent<MeshComponent>(
            vampireMesh, vampireMaterial, vampireTexture);
        vmc.UniformBuffer = vampireUniformBuffer;

        m_Framebuffer     = SceneRenderer::Framebuffer();
        UpdateDescriptorSets();
        m_SceneHierarchyPanel = AddPanel<SceneHierarchyPanel>(
            PanelCategory::eView, "SceneHierarchy", true, scene);
        IgnoreUnused(AddPanel<ContentBrowserPanel>(PanelCategory::eView,
                                                   "ContentBrowser", true));
        m_ConsolePanel
            = AddPanel<ConsolePanel>(PanelCategory::eView, "Console", true);

        m_IconPlay     = CreateRef<TextureAsset>("assets/icons/play.png");
        m_IconPause    = CreateRef<TextureAsset>("assets/icons/pause.png");
        m_IconStep     = CreateRef<TextureAsset>("assets/icons/step.png");
        m_IconStop     = CreateRef<TextureAsset>("assets/icons/stop.png");
        m_IconSimulate = CreateRef<TextureAsset>("assets/icons/simulate.png");

        auto& app      = *Application::Get();

        VtCoreTrace("Editor: Loading editorPreferences.json...");
        if (DeserializeJsonFromFile("editorPreferences.json", m_Preferences))
        {
            VtCoreInfo(
                "Editor: Successfully deserialized editorPreferences.json");
            OpenStartupProject();
        }
        else {
            VtCoreError("Editor: Failed to load editorPreferences.json");
        }
        Delegate<void(const ProjectChangedEvent&)> projectChanged;
        projectChanged.Bind<&EditorLayer::OnProjectChanged>(this);
        app.SubscribeEvent<ProjectChangedEvent>(projectChanged);

        Delegate<void(const MouseButtonPressedEvent&)> mousePressed;
        mousePressed.Bind<&EditorLayer::OnMouseButtonPressed>(this);
        app.SubscribeEvent<MouseButtonPressedEvent>(mousePressed);
    }
    void EditorLayer::OnDetach() {}

    void EditorLayer::OnUpdate(Timestep deltaTime)
    {
        VtProfileFunction();
        auto& window = m_App->Window();

        if (m_CompileState == ScriptCompilationState::eCompiling && m_BuildTask)
        {
            if (m_BuildTask->Poll() != ProcessState::eRunning)
            {
                ProcessResult result = m_BuildTask->Wait();
                m_CompileExitCode    = result.exitCode;

                if (result.exitCode == 0)
                {
                    m_CompileState  = ScriptCompilationState::eSuccess;
                    m_CompileOutput = result.stdoutOutput;
                    VtCoreInfo("Script Build Succeeded!\n{}", m_CompileOutput);

                    if (!ScriptEngine::ReloadAssembly())
                        VtCoreError(
                            "EditorLayer: Failed to reload the script "
                            "assembly!");
                }
                else {
                    m_CompileState  = ScriptCompilationState::eFailed;
                    m_CompileOutput = result.stderrOutput;
                    VtCoreError("Script Build Failed (Code {}):\n{}",
                                result.exitCode, m_CompileOutput);
                }

                m_BuildTask.Reset();
            }
        }
        if (!m_PendingScenePath.Empty())
        {
            Renderer::WaitIdle();

            auto& scene        = *Scene::Active();
            m_CurrentScenePath = m_PendingScenePath;
            DeserializeJsonFromFile(m_CurrentScenePath, scene);
            m_SceneHierarchyPanel->SetScene(scene);

            m_PendingScenePath = ""_p;
        }
        if (m_PendingProject)
        {
            Renderer::WaitIdle();
            EditorProject::SwitchTo(m_PendingProject);
            m_PendingProject = nullptr;
        }

        using namespace Input;
        if (m_ViewportFocused && m_SceneState == SceneState::eEdit)
            m_Camera.Update();
        Scene::Active()->Update(deltaTime);

        if (!m_ShouldResizeFramebuffer) return;
        m_Framebuffer->OnResize(static_cast<u32>(m_ViewportSize.x),
                                static_cast<u32>(m_ViewportSize.y));

        UpdateDescriptorSets();
        m_ShouldResizeFramebuffer = false;
    }
    void EditorLayer::OnRender()
    {
        Renderer::BeginRenderPass(m_Framebuffer);
        Scene::Active()->Render();
        Renderer::EndRenderPass();
    }
    void EditorLayer::OnImGuiRender()
    {
        VtProfileFunction();
        static bool               dockspaceOpen        = true;
        static bool               fullscreenPersistant = true;
        bool                      optFullscreen        = fullscreenPersistant;
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags          windowFlags
            = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (optFullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            windowFlags |= ImGuiWindowFlags_NoTitleBar
                         | ImGuiWindowFlags_NoCollapse
                         | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus
                         | ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
            windowFlags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Dockspace Demo", &dockspaceOpen, windowFlags);
        ImGui::PopStyleVar();
        if (optFullscreen) ImGui::PopStyleVar(2);

        auto& window = m_App->Window();
        auto& scene  = *Scene::Active();

        DrawMenuBar();
        DrawToolbar();

        ProjectConfig config;
        Path          projectPath;

        ImGuiIO&      io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }
        DrawStatisticsPanel();
        DrawSceneViewport();
        DrawGameViewport();

        m_PanelManager.OnImGuiRender();

        if (m_ThemeEditor)
        {
            ImGui::Begin("Theme Editor");
            ImGui::ShowStyleEditor();
            ImGui::End();
        }

        UI::ModalManager::RenderModals();

        ImGui::End();
        m_SettingsOverlay.Render(&m_ShowSettingsModal);

        frameIndex = Renderer::CurrentFrameIndex();
    }

    static AABB TransformAABB(const AABB& aabb, const Mat4& transform)
    {
        Vec3 corners[8] = {{aabb.Min.x, aabb.Min.y, aabb.Min.z},
                           {aabb.Max.x, aabb.Min.y, aabb.Min.z},
                           {aabb.Min.x, aabb.Max.y, aabb.Min.z},
                           {aabb.Max.x, aabb.Max.y, aabb.Min.z},
                           {aabb.Min.x, aabb.Min.y, aabb.Max.z},
                           {aabb.Max.x, aabb.Min.y, aabb.Max.z},
                           {aabb.Min.x, aabb.Max.y, aabb.Max.z},
                           {aabb.Max.x, aabb.Max.y, aabb.Max.z}};

        AABB transformedAABB;
        transformedAABB.Min = Vec3(FLT_MAX);
        transformedAABB.Max = Vec3(-FLT_MAX);

        for (const auto& corner : corners)
        {
            Vec3 transformed    = Vec3(transform * Vec4(corner, 1.0f));
            transformedAABB.Min = glm::min(transformedAABB.Min, transformed);
            transformedAABB.Max = glm::max(transformedAABB.Max, transformed);
        }

        return transformedAABB;
    }

    Math::Ray EditorLayer::CastRay(f32 mouseX, f32 mouseY, f32 viewportWidth,
                                   f32 viewportHeight, Camera& camera)
    {
        f32  ndcX = (2.0f * mouseX) / viewportWidth - 1.0f;
        f32  ndcY = (2.0f * mouseY) / viewportHeight - 1.0f;

        Vec4 rayStartClip(ndcX, ndcY, 0.0f, 1.0f);
        Vec4 rayEndClip(ndcX, ndcY, 1.0f, 1.0f);

        Mat4 invViewProj   = glm::inverse(camera.ViewProjection());

        Vec4 rayStartWorld = invViewProj * rayStartClip;
        rayStartWorld /= rayStartWorld.w;

        Vec4 rayEndWorld = invViewProj * rayEndClip;
        rayEndWorld /= rayEndWorld.w;

        Vec3 origin    = Vec3(rayStartWorld);
        Vec3 direction = glm::normalize(Vec3(rayEndWorld) - origin);

        return {origin, direction};
    }
    Actor EditorLayer::PickActor(const Math::Ray& ray, const Scene& scene)
    {
        Actor closestActor = {};
        f32   minHitT      = std::numeric_limits<f32>::max();

        // Iterate over ALL entities with a TransformComponent
        for (auto entity : scene.Registry().view<TransformComponent>())
        {
            Actor       actor(entity, const_cast<Scene&>(scene));
            const auto& transform = actor.GetComponent<TransformComponent>();

            AABB        localAABB;

            // 1. Use mesh AABB if available
            if (actor.HasComponent<MeshComponent>()
                && actor.GetComponent<MeshComponent>().Mesh)
                localAABB = actor.GetComponent<MeshComponent>().Mesh->AABB();
            // 2. Otherwise, fall back to a default bounding box
            else localAABB = AABB(Vec3(-0.5f), Vec3(0.5f));

            AABB worldAABB = TransformAABB(localAABB, Mat4(transform));

            f32  hitT      = 0.0f;
            if (ray.IntersectsAABB(worldAABB, hitT) && (hitT < minHitT))
            {
                minHitT      = hitT;
                closestActor = actor;
            }
        }

        return closestActor;
    }

    bool EditorLayer::OpenStartupProject()
    {
        auto startupProjectName = m_Preferences.StartupProject;
        if (startupProjectName.Empty()) return false;

        VtCoreTraceTag("Editor", "Opening the startup project '{}'",
                       startupProjectName);
        const auto& recentProjects = m_Preferences.RecentProjects;
        const auto  it             = std::find_if(
            recentProjects.begin(), recentProjects.end(), [&](const auto& pair)
            { return pair.second.Name == startupProjectName; });

        if (it == recentProjects.end())
        {
            VtCoreErrorTag("Editor",
                           "Failed to fetch startup project's entry in "
                           "editorPreferences.json");
            return false;
        }

        auto loaded = EditorProject::Load(it->second.Path);
        if (!loaded) return false;

        EditorProject::SwitchTo(loaded);
        return true;
    }
    void EditorLayer::BuildAssembly()
    {
        String projectPath
            = (std::filesystem::current_path() / "Sandbox" / "sandbox.csproj")
                  .string()
                  .data();
        auto buildProfile = "Debug";

        if (m_CompileState == ScriptCompilationState::eCompiling)
        {
            VtCoreWarnTag("Editor",
                          "Script compilation is already in progress.");
            return;
        }

        // Initialize background process
        m_BuildTask
            = Process::Create("dotnet", {"build", projectPath, "-c",
                                         buildProfile, "--no-incremental"});

        if (!m_BuildTask->Start())
        {
            m_CompileState  = ScriptCompilationState::eFailed;
            m_CompileOutput = "Failed to trigger dotnet compiler execution.";
            VtCoreError("{}", m_CompileOutput);
            return;
        }

        m_CompileState  = ScriptCompilationState::eCompiling;
        m_CompileOutput = "Compiling project scripts...";
        VtCoreTraceTag("Editor", "Triggered script compilation...");
    }

    void EditorLayer::UpdateDescriptorSets()
    {
        UI::RemoveTexture(m_FramebufferDescriptorSet);

        m_FramebufferDescriptorSet = UI::TextureFromFramebuffer(m_Framebuffer);
    }

    void EditorLayer::DrawMenuBar()
    {
        auto& scene = *Scene::Active();

        if (ImGui::BeginMainMenuBar())
        {
            // --- File Menu ---
            if (ImGui::BeginMenu("File"))
            {
                // --- Scene Operations ---
                if (ImGui::MenuItem("New Scene", "Ctrl+N"))
                    ;
                if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
                {
                    auto result     = Window::OpenFile(s_SceneFileFilter, ".");
                    const Path path = result ? *result : ""_p;

                    if (!path.Empty()) m_PendingScenePath = path;
                }
                if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                {
                    if (m_CurrentScenePath.Empty())
                    {
                        auto result = Window::SaveFile(s_SceneFileFilter);
                        if (result) m_CurrentScenePath = *result;
                    }

                    if (!m_CurrentScenePath.Empty())
                        SerializeJsonToFile(m_CurrentScenePath, scene);
                }
                if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
                {
                    auto       result = Window::SaveFile(s_SceneFileFilter);
                    const Path path   = result ? *result : ""_p;

                    if (!path.Empty())
                    {
                        SerializeJsonToFile(path, scene);
                        m_CurrentScenePath = path;
                    }
                }

                ImGui::Separator();

                // --- Project Operations ---
                if (ImGui::MenuItem("New Project", "CTRL+SHIFT+N"))
                    IgnoreUnused(UI::ModalManager::Open<NewProjectModal>(
                        [this](const ProjectConfig& config, PathView path,
                               bool generateGitIgnore)
                        {
                            auto newProject = EditorProject::New(config, path);
                            if (!newProject)
                            {
                                VtCoreError(
                                    "Editor: Failed to create new project!");
                                return;
                            }

                            m_PendingProject = newProject;
                        }));
                if (ImGui::MenuItem("Open Project", "CTRL+Alt+O"))
                {
                    auto result = Window::OpenFile(s_ProjectFileFilter, ".");
                    const Path path = result ? *result : ""_p;

                    if (!path.Empty())
                        m_PendingProject = EditorProject::Load(path);
                }
                if (ImGui::MenuItem("Save Project", "CTRL+Alt+S"))
                    ;
                if (ImGui::MenuItem("Save Project As", "CTRL+Alt+SHIFT+S"))
                    ;

                ImGui::Separator();
                if (ImGui::MenuItem("Reload", "Ctrl+R"))
                {
                    if (m_CurrentScenePath.Empty())
                    {
                        auto result = Window::OpenFile(s_SceneFileFilter, ".");
                        m_CurrentScenePath = result ? *result : ""_p;
                    }

                    if (!m_CurrentScenePath.Empty())
                    {
                        DeserializeJsonFromFile(m_CurrentScenePath, scene);
                        m_SceneHierarchyPanel->SetScene(scene);
                    }
                }
                if (ImGui::MenuItem("Restart", "Alt+F5")) m_App->Restart();
                if (ImGui::MenuItem("Quit", "Alt+F4")) m_App->Close();

                ImGui::EndMenu();
            }

            // --- Edit Menu ---
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}

                ImGui::Separator();
                m_PanelManager.ForEachInCategory(
                    PanelCategory::eEdit,
                    [](PanelDescriptor& data)
                    {
                        ImGui::MenuItem(data.Name.Raw(), nullptr, &data.IsOpen);
                    });
                ImGui::EndMenu();
            }

            // --- View Menu ---
            if (ImGui::BeginMenu("View"))
            {
                m_PanelManager.ForEachInCategory(
                    PanelCategory::eView,
                    [](PanelDescriptor& data)
                    {
                        ImGui::MenuItem(data.Name.Raw(), nullptr, &data.IsOpen);
                    });

                ImGui::EndMenu();
            }

            // --- Tools Menu ---
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Compile C# Scripts", "Ctrl+B", false,
                                    m_CompileState
                                        != ScriptCompilationState::eCompiling))
                    BuildAssembly();

                ImGui::Separator();

                m_PanelManager.ForEachInCategory(
                    PanelCategory::eTools,
                    [](PanelDescriptor& data)
                    {
                        ImGui::MenuItem(data.Name.Raw(), nullptr, &data.IsOpen);
                    });

                ImGui::Separator();
                if (ImGui::MenuItem("Settings", "Ctrl+Comma"))
                    m_ShowSettingsModal = true;

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
    void EditorLayer::DrawToolbar()
    {
        const float toolbarHeight = 32.0f;

        // Set styling variables for a flat look inside the child region
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto&       colors        = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        // Create a child frame block spanning full width and exact height
        // ImGuiWindowFlags_NoBackground leaves it matching the main engine
        // panel background color
        if (ImGui::BeginChild("##toolbar_zone", ImVec2(0, toolbarHeight), false,
                              ImGuiWindowFlags_NoScrollbar
                                  | ImGuiWindowFlags_NoScrollWithMouse))
        {
            constexpr bool toolbarEnabled = true;
            ImVec4         tintColor      = ImVec4(1, 1, 1, 1);
            if (!toolbarEnabled) tintColor.w = 0.5f;

            float buttonSize     = toolbarHeight - 8.0f;

            // Count visible elements to compute centering offset
            int   visibleButtons = 0;
            bool  hasPlayButton  = m_SceneState == SceneState::eEdit
                              || m_SceneState == SceneState::ePlay;
            bool hasSimulateButton = m_SceneState == SceneState::eEdit
                                  || m_SceneState == SceneState::eSimulate;
            bool hasPauseButton = m_SceneState != SceneState::eEdit;

            if (hasPlayButton) visibleButtons++;
            if (hasSimulateButton) visibleButtons++;
            if (hasPauseButton)
            {
                visibleButtons++;
                visibleButtons++;
            }

            float spacing    = ImGui::GetStyle().ItemSpacing.x;
            float totalWidth = (buttonSize * visibleButtons)
                             + (spacing * (visibleButtons - 1));

            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalWidth)
                                 * 0.5f);

            if (hasPlayButton)
            {
                ::Ref<Texture2D> icon
                    = (m_SceneState == SceneState::eEdit
                       || m_SceneState == SceneState::eSimulate)
                        ? m_IconPlay->Texture()
                        : m_IconStop->Texture();
                if (ImGui::ImageButton(
                        "##PlayButton",
                        (ImTextureID)(uint64_t)UI::GetTexture(icon),
                        ImVec2(buttonSize, buttonSize), ImVec2(0, 0),
                        ImVec2(1, 1), ImVec4(0, 0, 0, 0), tintColor))
                {
                    if (m_SceneState == SceneState::eEdit
                        || m_SceneState == SceneState::eSimulate)
                    {
                        auto& scene = *Scene::Active();
                        scene.OnRuntimeStart();
                        VtCoreTrace("OnScenePlay");
                    }
                    else if (m_SceneState == SceneState::ePlay)
                    {
                        VtCoreTrace("OnSceneStop");
                    }
                }
            }

            if (hasSimulateButton)
            {
                if (hasPlayButton) ImGui::SameLine();
                ::Ref<Texture2D> icon = (m_SceneState == SceneState::eEdit
                                         || m_SceneState == SceneState::ePlay)
                                          ? m_IconSimulate->Texture()
                                          : m_IconStop->Texture();
                if (ImGui::ImageButton(
                        "##SimulateButton",
                        (ImTextureID)(uint64_t)UI::GetTexture(icon),
                        ImVec2(buttonSize, buttonSize), ImVec2(0, 0),
                        ImVec2(1, 1), ImVec4(0, 0, 0, 0), tintColor))
                {
                    if (m_SceneState == SceneState::eEdit
                        || m_SceneState == SceneState::ePlay)
                    {
                        VtCoreTrace("OnSceneSimulate");
                    }
                    else if (m_SceneState == SceneState::eSimulate)
                    {
                        VtCoreTrace("OnSceneStop");
                    }
                }
            }

            if (hasPauseButton)
            {
                constexpr bool isPaused = true;
                ImGui::SameLine();
                if (ImGui::ImageButton("##PauseButton",
                                       (ImTextureID)(uint64_t)UI::GetTexture(
                                           m_IconPause->Texture()),
                                       ImVec2(buttonSize, buttonSize),
                                       ImVec2(0, 0), ImVec2(1, 1),
                                       ImVec4(0, 0, 0, 0), tintColor))
                {
                    VtCoreTrace("SetPaused");
                }

                if (isPaused)
                {
                    ImGui::SameLine();
                    if (ImGui::ImageButton(
                            "##StepButton",
                            (ImTextureID)(uint64_t)UI::GetTexture(
                                m_IconStep->Texture()),
                            ImVec2(buttonSize, buttonSize), ImVec2(0, 0),
                            ImVec2(1, 1), ImVec4(0, 0, 0, 0), tintColor))
                    {
                        VtCoreTrace("OnSceneStep");
                    }
                }
            }

            ImGui::EndChild();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }
    void EditorLayer::DrawStatisticsPanel()
    {
        ImGui::Begin("Statistics");
        ImGui::Text("FPS: %lu", m_App->FPSCounter());
        ImGui::Text("Delta Time: %f", m_App->DeltaTime());

        ImGui::Text("VRAM: %luMB/%luMB",
                    Renderer::MemoryUsage() / 1024ul / 1024ul,
                    Renderer::MemoryBudget() / 1024ul / 1024ul);
        ImGui::Text("Memory Budget: %luMB",
                    Renderer::MemoryBudget() / 1024ul / 1024ul);

        static bool demoWindow = false;
        ImGui::Checkbox("Demo Window", &demoWindow);
        if (demoWindow) ImGui::ShowDemoWindow(&demoWindow);

        auto&       window = m_App->Window();
        static bool vsync  = false;
        if (ImGui::Checkbox("VSync", &vsync))
            window.SwapChain()->SetVSync(vsync);

        static f32 fov         = 75.0f;
        static f32 aspectRatio = static_cast<f32>(window.Width())
                               / static_cast<f32>(window.Height());
        static f32 planes[] = {0.1f, 100.0f};

        if (ImGui::DragFloat("fov", &fov, 0.1, 45.0f, 145.0f)
            || ImGui::DragFloat2("Near/Far planes", planes, 0.1f, 0.1f))
            m_Camera.SetPerspective(glm::radians(fov), aspectRatio, planes[0],
                                    planes[1]);
        if (ImGui::Button("flip Y"))
        {
            auto projection = m_Camera.Projection();
            projection[1][1] *= -1;

            m_Camera.SetProjection(projection);
        }

        ImGui::End();
    }
    void EditorLayer::DrawSceneViewport()
    {
        ImGui::Begin("Viewport");
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();

        ImVec2 minBound   = ImGui::GetWindowPos();
        ImVec2 maxBound   = {minBound.x + ImGui::GetWindowSize().x,
                             minBound.y + ImGui::GetWindowSize().y};

        ImVec2 viewportOffset
            = ImGui::GetCursorStartPos(); // Account for tab bar / padding
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        m_ViewportBounds[0]  = {minBound.x, minBound.y};
        m_ViewportBounds[1]  = {maxBound.x, maxBound.y};

        auto framebufferSize = m_Framebuffer->Size();
        auto viewportSize    = ImGui::GetContentRegionAvail();
        if (viewportSize.x <= 0) viewportSize.x = 1;
        if (viewportSize.y <= 0) viewportSize.y = 1;

        m_ViewportSize = {viewportSize.x, viewportSize.y};

        if (framebufferSize.x != static_cast<u32>(m_ViewportSize.x)
            || framebufferSize.y != static_cast<u32>(m_ViewportSize.y))
            m_ShouldResizeFramebuffer = true;

        ImGui::Image(m_FramebufferDescriptorSet,
                     ImVec2(framebufferSize.x, framebufferSize.y));
        Actor selectedActor = m_SceneHierarchyPanel->SelectedActor();
        if (selectedActor && selectedActor.HasComponent<TransformComponent>())
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            f32 windowWidth  = static_cast<f32>(ImGui::GetWindowWidth());
            f32 windowHeight = static_cast<f32>(ImGui::GetWindowHeight());
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                              windowWidth, windowHeight);

            auto& tc         = selectedActor.GetComponent<TransformComponent>();
            Mat4  transform  = tc;

            auto  cameraView = m_Camera.View();
            auto  cameraProjection = m_Camera.Projection();
            cameraProjection[1][1] *= -1;
            transform[1][1] *= -1;
            ImGuizmo::Manipulate(
                glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL,
                glm::value_ptr(transform));

            // TODO(v1tr10l7): this only works for translation, need to
            // implement rotation and scale as well
            if (ImGuizmo::IsUsing()) tc.Translation = Vec3(transform[3]);
        }

        ImGui::End();
    }
    void EditorLayer::DrawGameViewport() {}

    void EditorLayer::OnProjectChanged(const ProjectChangedEvent& e)
    {
        auto&         project        = *e.Project;

        auto          runtimeProject = project.RuntimeProject();
        auto&         config         = runtimeProject->Config();

        auto          projectName    = config.Metadata.Name;
        RecentProject recent = {projectName, project.ConfigPath(), e.Time};

        m_Preferences.InsertRecentProject(recent);

        // TODO(v1tr10l7): better path?
        Path editorPreferencesPath = "editorPreferences.json";
        SerializeJsonToFile(editorPreferencesPath, m_Preferences);
    }
    void EditorLayer::OnMouseButtonPressed(const MouseButtonPressedEvent& e)
    {
        if (m_SceneState == SceneState::eEdit
            && (!m_ViewportHovered || ImGuizmo::IsOver()
                || e.Button != Input::MouseCode::eLeft || !Scene::Active()))
            return;
        ImGui::ClearActiveID();

        // Convert mouse position from global screen space to local viewport
        // relative coordinates
        auto [mouseX, mouseY] = ImGui::GetMousePos();

        mouseX -= m_ViewportBounds[0].x;
        mouseY -= m_ViewportBounds[0].y;

        f32 viewportWidth  = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
        f32 viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;
        // Ensure the mouse is strictly within the panel bounds before
        // raycasting
        if (mouseX >= 0.0f && mouseY >= 0.0f && mouseX < viewportWidth
            && mouseY < viewportHeight)
        {
            auto  ray = CastRay(mouseX, mouseY, viewportWidth, viewportHeight,
                                m_Camera);
            Actor pickedActor = PickActor(ray, *Scene::Active());
            if (pickedActor)
                m_SceneHierarchyPanel->SetSelectedActor(pickedActor);
        }
    }
}; // namespace Vortex::Editor
