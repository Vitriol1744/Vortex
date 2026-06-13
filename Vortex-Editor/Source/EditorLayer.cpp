/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <EditorLayer.hpp>

#include <imgui.h>

#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Engine/Application.hpp>

#include <Vortex/Renderer/API/Vulkan/VulkanFramebuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp>
#include <Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Renderer/SceneRenderer.hpp>
#include <Vortex/Scene/Components.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>
#include <Vortex/Window/Input/Mouse.hpp>

namespace Vortex
{
    // TODO(v1tr10l7): Shouldn't be defined here
    using PipelineSpecification = GraphicsPipelineSpecification;
    using Pipeline              = GraphicsPipeline;

    static u32 frameIndex       = 0;
    void       EditorLayer::OnAttach()
    {
        VtProfileFunction();
        Window& window = m_App->Window();
        // TODO(v1tr10l7): Should be handled by the asset manager or something
        Image   image("assets/icons/icon.bmp");

        window.SetIcon(image);
        window.ShowCursor();

        m_Shader = SceneRenderer::Shader();

        // TODO(v1tr10l7): this is just for testing,
        auto vikingRoomTexture
            = Texture2D::Create("assets/textures/viking_room.png");
        auto vampireTexture = Texture2D::Create("assets/textures/vampire.png");
        auto vikingRoomMesh = CreateRef<Mesh>("assets/models/viking_room.obj");

        auto vikingRoomEntity = m_Scene.AddEntity();
        vikingRoomEntity.AddComponent<TagComponent>("Viking Room");

        auto vikingRoomMaterial = Material::Create(m_Shader);
        auto vikingRoomUniformBuffer
            = UniformBuffer::Create(sizeof(UniformBufferObject));
        vikingRoomMaterial->Set("UniformBufferObject", vikingRoomUniformBuffer);
        vikingRoomMaterial->Set("texSampler", vikingRoomTexture);
        auto& vrmc = vikingRoomEntity.AddComponent<MeshComponent>(
            vikingRoomMesh, vikingRoomMaterial, vikingRoomTexture);
        vrmc.UniformBuffer = vikingRoomUniformBuffer;
        auto vampireEntity = m_Scene.AddEntity();
        vampireEntity.AddComponent<TagComponent>("Dancing Vampire");

        auto vampireMesh = CreateRef<Mesh>("assets/models/dancing_vampire.dae");
        auto vampireMaterial = Material::Create(m_Shader);
        auto vampireUniformBuffer
            = UniformBuffer::Create(sizeof(UniformBufferObject));
        vampireMaterial->Set("UniformBufferObject", vampireUniformBuffer);
        vampireMaterial->Set("texSampler", vampireTexture);
        auto& vmc = vampireEntity.AddComponent<MeshComponent>(
            vampireMesh, vampireMaterial, vampireTexture);
        vmc.UniformBuffer = vampireUniformBuffer;

        m_Framebuffer     = SceneRenderer::Framebuffer();
        UpdateDescriptorSets();
        m_CurrentScenePath = "assets/scenes/default-scene.vproj";
        m_Scene.SetName("Vortex-Scene");
        m_Scene.Deserialize(m_CurrentScenePath);
        m_Scene.SetMainCamera(m_Camera);
        m_Panels.EmplaceBack(m_SceneHierarchyPanel
                             = CreateRef<SceneHierarchyPanel>(m_Scene));
        m_Panels.EmplaceBack(CreateRef<ContentBrowserPanel>());
    }
    void EditorLayer::OnDetach()
    {
        // TODO(v1tr10l7): Abstract away vulkan stuff
        // if (m_FramebufferDescriptorSet)
        //     ImGui_ImplVulkan_RemoveTexture(
        //         (VkDescriptorSet)m_FramebufferDescriptorSet);
        // m_FramebufferDescriptorSet = nullptr;
    }

    void EditorLayer::OnUpdate(Timestep deltaTime)
    {
        VtProfileFunction();
        auto& window = m_App->Window();

        using namespace Input;
        if (m_ViewportFocused) m_Camera.Update();
        m_Scene.Update(deltaTime);

        if (!m_ShouldResizeFramebuffer) return;
        m_Framebuffer->OnResize(static_cast<u32>(m_ViewportSize.x),
                                static_cast<u32>(m_ViewportSize.y));

        UpdateDescriptorSets();
        m_ShouldResizeFramebuffer = false;
    }
    void EditorLayer::OnRender()
    {
        Renderer::BeginRenderPass(m_Framebuffer);
        m_Scene.Render();
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

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }

        Window& window = m_App->Window();
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    if (m_CurrentScenePath.Empty())
                        m_CurrentScenePath = window.SaveFileDialog();

                    if (!m_CurrentScenePath.Empty())
                        m_Scene.Serialize(m_CurrentScenePath);
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                {
                    const Path path = window.SaveFileDialog();

                    if (!path.Empty())
                    {
                        m_Scene.Serialize(path);
                        m_CurrentScenePath = path;
                    }
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    const Path path = window.OpenFileDialog(".");

                    if (!path.Empty())
                    {
                        m_CurrentScenePath = path;
                        m_Scene.Deserialize(path);
                        m_SceneHierarchyPanel->SetScene(m_Scene);
                    }
                }
                if (ImGui::MenuItem("Reload", "Ctrl+R"))
                {
                    if (m_CurrentScenePath.Empty())
                        m_CurrentScenePath = window.OpenFileDialog(".");
                    if (!m_CurrentScenePath.Empty())
                    {
                        m_Scene.Deserialize(m_CurrentScenePath);
                        m_SceneHierarchyPanel->SetScene(m_Scene);
                    }
                }
                if (ImGui::MenuItem("Restart", "Alt+F5")) m_App->Restart();
                if (ImGui::MenuItem("Quit", "Alt+F4")) m_App->Close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::Separator();
                for (auto& panel : m_Panels)
                    ImGui::Checkbox(panel->Name().Raw(), &panel->Enabled);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        DrawStatisticsPanel();
        DrawViewport();

        for (auto& panel : m_Panels) panel->Draw();
        ImGui::End();

        frameIndex = Renderer::CurrentFrameIndex();

        ::Ref<VulkanFramebuffer> vkFramebuffer
            = m_Framebuffer.As<VulkanFramebuffer>();
    }

    void EditorLayer::UpdateDescriptorSets()
    {
        // TODO(v1tr10l7): Abstract away vulkan stuff
        if (m_FramebufferDescriptorSet)
            ImGui_ImplVulkan_RemoveTexture(
                static_cast<VkDescriptorSet>(m_FramebufferDescriptorSet));

        auto vkFramebuffer         = m_Framebuffer.As<VulkanFramebuffer>();
        m_FramebufferDescriptorSet = ImGui_ImplVulkan_AddTexture(
            vkFramebuffer->Sampler(), vkFramebuffer->ImageView(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

        Window&     window = m_App->Window();
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
    void EditorLayer::DrawViewport()
    {
        ImGui::Begin("Viewport");
        m_ViewportFocused    = ImGui::IsWindowFocused();
        m_ViewportHovered    = ImGui::IsWindowHovered();

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
        Entity selectedEntity = m_SceneHierarchyPanel->SelectedEntity();
        if (selectedEntity && selectedEntity.HasComponent<TransformComponent>())
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            f32 windowWidth  = static_cast<f32>(ImGui::GetWindowWidth());
            f32 windowHeight = static_cast<f32>(ImGui::GetWindowHeight());
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                              windowWidth, windowHeight);

            auto&     tc = selectedEntity.GetComponent<TransformComponent>();
            glm::mat4 transform        = tc;

            auto      cameraView       = m_Camera.View();
            auto      cameraProjection = m_Camera.Projection();
            cameraProjection[1][1] *= -1;
            transform[1][1] *= -1;
            ImGuizmo::Manipulate(
                glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL,
                glm::value_ptr(transform));

            // TODO(v1tr10l7): this only works for translation, need to
            // implement rotation and scale as well
            if (ImGuizmo::IsUsing()) tc.Translation = glm::vec3(transform[3]);
        }

        ImGui::End();
    }
}; // namespace Vortex
