/*
 * Created by v1tr10l7 on 07.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "EditorLayer.hpp"

#include <imgui.h>

#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Engine/Application.hpp>

#include <Vortex/Renderer/API/Vulkan/VulkanFramebuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp>
#include <Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h>
#include <Vortex/Renderer/Renderer.hpp>
#include <Vortex/Renderer/Renderer2D.hpp>
#include <Vortex/Window/Input/Keyboard.hpp>

namespace Vortex
{
    using PipelineSpecification = GraphicsPipelineSpecification;
    using Pipeline              = GraphicsPipeline;

    static u32 frameIndex       = 0;

    void       EditorLayer::OnAttach()
    {
        VtProfileFunction();
        Window& window = Application::Get()->GetWindow();
        window.SetPosition(300, 300);

        Image image("assets/icon.bmp");

        window.SetIcon(image);
        window.ShowCursor();

        m_Shader = Shader::Create("assets/shaders/texture.glsl");
        std::initializer_list<VertexBufferElement> elements
            = {ShaderDataType::eFloat3, ShaderDataType::eFloat3,
               ShaderDataType::eFloat2};
        VertexBufferLayout    layout(elements);

        PipelineSpecification pipelineSpecs{};
        pipelineSpecs.Shader = m_Shader;
        pipelineSpecs.Window = &Application::Get()->GetWindow();
        pipelineSpecs.Layout = layout;
        m_Pipeline           = Pipeline::Create(pipelineSpecs);
        m_Texture            = Texture2D::Create("assets/textures/texture.jpg");

        auto vulkanTexture
            = std::dynamic_pointer_cast<VulkanTexture2D>(m_Texture);
        vk::Sampler   sampler   = vulkanTexture->GetSampler();
        vk::ImageView imageView = vulkanTexture->GetImageView();
        m_DescriptorSet         = ImGui_ImplVulkan_AddTexture(
            sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        FramebufferSpecification framebufferSpecs{};
        framebufferSpecs.SwapChainTarget = false;
        framebufferSpecs.Width           = 1280;
        framebufferSpecs.Height          = 800;

        m_Framebuffer         = Framebuffer::Create(framebufferSpecs);
        u32 maxFramesInFlight = Renderer::GetConfiguration().MaxFramesInFlight;
        m_FramebufferDescriptorSets.resize(maxFramesInFlight);
        for (u32 i = 0; i < maxFramesInFlight; i++)
        {
            Ref<VulkanFramebuffer> vkFramebuffer
                = std::dynamic_pointer_cast<VulkanFramebuffer>(m_Framebuffer);
            vk::Sampler framebufferSampler
                = vkFramebuffer->GetFrames()[i].Sampler;
            vk::ImageView framebufferImageView
                = vkFramebuffer->GetFrames()[i].ImageView;

            m_FramebufferDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
                framebufferSampler, framebufferImageView,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        m_CurrentScenePath = "assets/scenes/default-scene.vproj";
        m_Scene.SetName("Vortex-Scene");
        m_Scene.Deserialize(m_CurrentScenePath);
        m_SceneHierarchyPanel = CreateRef<SceneHierarchyPanel>(m_Scene);
    }
    void EditorLayer::OnDetach()
    {
        // TODO(v1tr10l7): delete textures
    }

    void EditorLayer::OnUpdate()
    {
        VtProfileFunction();

        auto& window = Application::Get()->GetWindow();

        using namespace Input;
        if (m_ViewportFocused) m_Camera.Update();

        if (m_ShouldResizeFramebuffer)
        {
            m_Framebuffer->OnResize(static_cast<u32>(m_ViewportSize.x),
                                    static_cast<u32>(m_ViewportSize.y));

            Ref<VulkanFramebuffer> vkFramebuffer
                = std::dynamic_pointer_cast<VulkanFramebuffer>(m_Framebuffer);

            for (u32 i = 0; i < Renderer::GetConfiguration().MaxFramesInFlight;
                 i++)
            {
                vk::Sampler framebufferSampler
                    = vkFramebuffer->GetFrames()[frameIndex].Sampler;
                vk::ImageView framebufferImageView
                    = vkFramebuffer->GetFrames()[frameIndex].ImageView;

                ImGui_ImplVulkan_RemoveTexture(
                    (VkDescriptorSet)m_FramebufferDescriptorSets[i]);

                m_FramebufferDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(
                    framebufferSampler, framebufferImageView,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }

            m_ShouldResizeFramebuffer = false;
        }
    }
    void EditorLayer::OnRender()
    {
        Renderer::BeginRenderPass(m_Framebuffer);

        Renderer2D::BeginScene(m_Camera);
        m_Scene.DrawEntities();
        Renderer2D::EndScene();

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

        Window& window = Application::Get()->GetWindow();
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    if (m_CurrentScenePath.empty())
                        m_CurrentScenePath = window.SaveFileDialog();

                    if (!m_CurrentScenePath.empty())
                        m_Scene.Serialize(m_CurrentScenePath);
                }
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                {
                    const Path path = window.SaveFileDialog();

                    if (!path.empty())
                    {
                        m_Scene.Serialize(path);
                        m_CurrentScenePath = path;
                    }
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    const Path path = window.OpenFileDialog(".");

                    if (!path.empty())
                    {
                        m_CurrentScenePath = path;
                        m_Scene.Deserialize(path);
                        m_SceneHierarchyPanel->SetScene(m_Scene);
                    }
                }
                if (ImGui::MenuItem("Restart", "Alt+F5"))
                    Application::Get()->Restart();
                if (ImGui::MenuItem("Quit", "Alt+F4"))
                    Application::Get()->Close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::Separator();
                ImGui::Checkbox(m_SceneHierarchyPanel->GetName().data(),
                                &m_SceneHierarchyPanel->Enabled);

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        DrawStatisticsPanel();
        DrawViewport();
        m_SceneHierarchyPanel->Draw();

        ImGui::End();

        frameIndex = Renderer::GetCurrentFrameIndex();

        Ref<VulkanFramebuffer> vkFramebuffer
            = std::dynamic_pointer_cast<VulkanFramebuffer>(m_Framebuffer);
        vkFramebuffer->SetCurrentImageIndex(frameIndex);
    }

    void EditorLayer::DrawStatisticsPanel()
    {
        ImGui::Begin("Statistics");
        ImGui::Text("FPS: %lu", Application::Get()->GetFPSCounter());
        ImGui::Text("Delta Time: %f", Application::Get()->GetDeltaTime());

        ImGui::Text("VRAM: %luMB/%luMB",
                    Renderer::GetMemoryUsage() / 1024ul / 1024ul,
                    Renderer::GetMemoryBudget() / 1024ul / 1024ul);
        ImGui::Text("Memory Budget: %luMB",
                    Renderer::GetMemoryBudget() / 1024ul / 1024ul);

        ImGui::Image(m_DescriptorSet, ImVec2(200.0f, 200.0f));

        static bool demoWindow = false;
        ImGui::Checkbox("Demo Window", &demoWindow);
        if (demoWindow) ImGui::ShowDemoWindow(&demoWindow);

        Window&     window = Application::Get()->GetWindow();
        static bool vsync  = false;
        if (ImGui::Checkbox("VSync", &vsync))
            window.GetSwapChain()->SetVSync(vsync);

        static f32 fov         = 75.0f;
        static f32 aspectRatio = static_cast<f32>(window.GetWidth())
                               / static_cast<f32>(window.GetHeight());
        static f32 planes[] = {0.1f, 100.0f};

        if (ImGui::DragFloat("fov", &fov)
            || ImGui::DragFloat2("Near/Far planes", planes))
            m_Camera.SetPerspective(glm::radians(fov), aspectRatio, planes[0],
                                    planes[1]);
        if (ImGui::Button("flip Y"))
        {
            auto projection = m_Camera.GetProjection();
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

        auto framebufferSize = m_Framebuffer->GetSize();
        auto viewportSize    = ImGui::GetContentRegionAvail();
        if (viewportSize.x <= 0) viewportSize.x = 1;
        if (viewportSize.y <= 0) viewportSize.y = 1;

        m_ViewportSize = {viewportSize.x, viewportSize.y};

        if (framebufferSize.x != static_cast<u32>(m_ViewportSize.x)
            || framebufferSize.y != static_cast<u32>(m_ViewportSize.y))
            m_ShouldResizeFramebuffer = true;

        ImGui::Image(m_FramebufferDescriptorSets[frameIndex],
                     ImVec2(framebufferSize.x, framebufferSize.y));
        Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
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

            auto      cameraView       = m_Camera.GetView();
            auto      cameraProjection = m_Camera.GetProjection();
            cameraProjection[1][1] *= -1;
            transform[1][1] *= -1;
            ImGuizmo::Manipulate(
                glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL,
                glm::value_ptr(transform));

            if (ImGuizmo::IsUsing()) tc.Translation = glm::vec3(transform[3]);
        }

        ImGui::End();
    }
}; // namespace Vortex
