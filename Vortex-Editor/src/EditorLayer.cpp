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
    static Camera s_Camera;

    static u32    frameIndex      = 0;
    static f32    s_MovementSpeed = 0.5f;
    static Vec3   s_Translation   = glm::vec3(0.0f, 0.0f, 0.0f);

    static f32    s_ZoomLevel     = 3.0f;

    void          EditorLayer::OnAttach()
    {
        VtProfileFunction();
        Window& window = Application::Get()->GetWindow();
        window.SetPosition(300, 300);

        static auto onMouseScrolled = [this](Window*, f64, f64 deltaY) -> bool
        {
            if (!m_ViewportHovered) return false;

            s_ZoomLevel += deltaY * Application::Get()->GetDeltaTime();
            return false;
        };

        WindowEvents::MouseScrolledEvent += onMouseScrolled;

        Image image("assets/icon.bmp");

        window.SetIcon(image);
        window.ShowCursor();

        m_Shader = Shader::Create("assets/shaders/texture.glsl");
        std::initializer_list<VertexBufferElement> elements
            = {ShaderDataType::eFloat3, ShaderDataType::eFloat3,
               ShaderDataType::eFloat2};
        VertexBufferLayout            layout(elements);

        GraphicsPipelineSpecification pipelineSpecs{};
        pipelineSpecs.Shader = m_Shader;
        pipelineSpecs.Window = &Application::Get()->GetWindow();
        pipelineSpecs.Layout = layout;
        m_Pipeline           = GraphicsPipeline::Create(pipelineSpecs);
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

        f32 aspectRatio = 800.0f / 600.0f;
        f32 zoomLevel   = 3.0f;
        s_Camera.SetOrthographic(aspectRatio * zoomLevel,
                                 -aspectRatio * zoomLevel, zoomLevel,
                                 -zoomLevel, 0.0f, 1.0f);

        Vec3 scale(0.05f, 0.05f, 0.5f);
        u32  i = 0;
        for (f32 y = -5.0f; y < 5.0f; y += 0.5f)
        {
            for (f32 x = -5.0f; x < 5.0f; x += 0.5f)
            {
                auto  quad         = m_Scene.AddEntity();
                auto& tagComponent = quad.AddComponent<TagComponent>();
                auto& transformComponent
                    = quad.AddComponent<TransformComponent>();
                auto& spriteComponent = quad.AddComponent<SpriteComponent>();

                Vec2  pos(x * 0.11f, y * 0.11f);
                transformComponent.Translation = Vec3(pos, 0.0f);
                transformComponent.Scale       = scale;
                transformComponent.Scale       = scale;
                transformComponent.Rotation    = glm::quat();

                tagComponent.Name              = fmt::format("Quad #{}", i);
                spriteComponent.Color
                    = Vec4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.2f);
                ++i;
            }
        }

        m_Panels.push_back(CreateScope<SceneHierarchyPanel>(m_Scene));
    }
    void EditorLayer::OnDetach()
    {
        // TODO(v1tr10l7): delete textures
    }

    void EditorLayer::OnUpdate()
    {
        VtProfileFunction();

        auto& window      = Application::Get()->GetWindow();
        f32   aspectRatio = static_cast<f32>(window.GetWidth())
                        / static_cast<f32>(window.GetHeight());
        s_ZoomLevel = std::clamp(s_ZoomLevel, 0.0001f, 20.0f);
        s_Camera.SetOrthographic(aspectRatio * s_ZoomLevel,
                                 -aspectRatio * s_ZoomLevel, s_ZoomLevel,
                                 -s_ZoomLevel, 0.0f, 1.0f);

        using namespace Input;
        if (m_ViewportFocused)
        {
            if (Keyboard::IsKeyPressed(KeyCode::eA))
                s_Translation.x
                    += s_MovementSpeed * Application::Get()->GetDeltaTime();
            else if (Keyboard::IsKeyPressed(KeyCode::eD))
                s_Translation.x
                    -= s_MovementSpeed * Application::Get()->GetDeltaTime();
            else if (Keyboard::IsKeyPressed(KeyCode::eW))
                s_Translation.y
                    -= s_MovementSpeed * Application::Get()->GetDeltaTime();
            else if (Keyboard::IsKeyPressed(KeyCode::eS))
                s_Translation.y
                    += s_MovementSpeed * Application::Get()->GetDeltaTime();
        }

        s_Camera.SetPosition(s_Translation);

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

        Renderer2D::BeginScene(s_Camera);

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

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Save", "Ctrl+S")) m_Scene.Serialize();
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                    m_Scene.Serialize();
                if (ImGui::MenuItem("Restart", "Alt+F5"))
                    Application::Get()->Restart();
                if (ImGui::MenuItem("Quit", "Alt+F4"))
                    Application::Get()->Close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::Separator();
                for (auto& panel : m_Panels)
                    ImGui::Checkbox(panel->GetName().data(), &panel->Enabled);

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        DrawStatisticsPanel();
        DrawViewport();
        for (auto& panel : m_Panels) panel->Draw();

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

        static bool vsync = false;
        if (ImGui::Checkbox("VSync", &vsync))
            Application::Get()->GetWindow().GetSwapChain()->SetVSync(vsync);

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
        ImGui::End();
    }
}; // namespace Vortex
