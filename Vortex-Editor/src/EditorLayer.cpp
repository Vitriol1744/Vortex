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
#include <Vortex/Window/Input/Mouse.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Vortex
{
    using PipelineSpecification           = GraphicsPipelineSpecification;
    using Pipeline                        = GraphicsPipeline;

    static u32                 frameIndex = 0;

    static std::vector<Vertex> vertices;
    static std::vector<u32>    indices;

    void                       ProcessMesh(aiMesh* mesh)
    {
        for (usize i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex{};
            vertex.Pos.x = mesh->mVertices[i].x;
            vertex.Pos.y = mesh->mVertices[i].y;
            vertex.Pos.z = mesh->mVertices[i].z;
            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            }
            vertex.Color = Vec3(1.0f, 1.0f, 1.0f);
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
    }

    void ProcessNode(aiNode* node, const aiScene* scene)
    {
        for (usize i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh);
        }
        for (usize i = 0; i < node->mNumChildren; i++)
            ProcessNode(node->mChildren[i], scene);
    }

    void EditorLayer::OnAttach()
    {
        VtProfileFunction();
        Window& window = m_App->GetWindow();
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
        pipelineSpecs.Window = &m_App->GetWindow();
        pipelineSpecs.Layout = layout;
        m_Pipeline           = Pipeline::Create(pipelineSpecs);
        m_Textures.push_back(
            Texture2D::Create("assets/textures/viking_room.png"));
        m_Textures.push_back(Texture2D::Create("assets/textures/vampire.png"));
        m_UniformBuffer = UniformBuffer::Create(sizeof(UniformBufferObject));
        m_Meshes.push_back(CreateRef<Mesh>("assets/models/viking_room.obj"));
        m_Meshes.push_back(
            CreateRef<Mesh>("assets/models/dancing_vampire.dae"));

        auto vulkanTexture
            = std::dynamic_pointer_cast<VulkanTexture2D>(m_Textures[0]);
        vk::Sampler   sampler   = vulkanTexture->GetSampler();
        vk::ImageView imageView = vulkanTexture->GetImageView();
        m_DescriptorSet         = ImGui_ImplVulkan_AddTexture(
            sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        FramebufferSpecification framebufferSpecs{};
        framebufferSpecs.SwapChainTarget = false;
        framebufferSpecs.Width           = 1280;
        framebufferSpecs.Height          = 800;

        m_Materials.push_back(Material::Create(m_Shader));
        m_Materials.push_back(Material::Create(m_Shader));
        m_Materials[0]->Set("UniformBufferObject", m_UniformBuffer);
        m_Materials[0]->Set("texSampler", m_Textures[0]);
        m_Materials[1]->Set("UniformBufferObject", m_UniformBuffer);
        m_Materials[1]->Set("texSampler", m_Textures[1]);
        m_Shader->SetUniform("UniformBufferObject", m_UniformBuffer);
        m_Shader->SetUniform("texSampler", m_Textures[0]);

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

        auto& window = m_App->GetWindow();

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
        for (u32 i = 0; i < m_Meshes.size(); i++)
        {
            Ref<Mesh>      mesh    = m_Meshes[i];
            Ref<Texture2D> texture = m_Textures[i];

            Renderer::Draw(m_Pipeline, mesh->GetVertexBuffer(),
                           mesh->GetIndexBuffer(), m_Materials[i]);
        }
        UniformBufferObject ubo{};
        ubo.Model          = Mat4(1.0f);
        ubo.ViewProjection = m_Camera.GetViewProjection();
        m_UniformBuffer->SetData(&ubo, sizeof(ubo), 0);
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

        Window& window = m_App->GetWindow();
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
                if (ImGui::MenuItem("Reload", "Ctrl+R"))
                {
                    if (m_CurrentScenePath.empty())
                        m_CurrentScenePath = window.OpenFileDialog(".");
                    if (!m_CurrentScenePath.empty())
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
        ImGui::Text("FPS: %lu", m_App->GetFPSCounter());
        ImGui::Text("Delta Time: %f", m_App->GetDeltaTime());

        ImGui::Text("VRAM: %luMB/%luMB",
                    Renderer::GetMemoryUsage() / 1024ul / 1024ul,
                    Renderer::GetMemoryBudget() / 1024ul / 1024ul);
        ImGui::Text("Memory Budget: %luMB",
                    Renderer::GetMemoryBudget() / 1024ul / 1024ul);

        ImGui::Image(m_DescriptorSet, ImVec2(200.0f, 200.0f));

        static bool demoWindow = false;
        ImGui::Checkbox("Demo Window", &demoWindow);
        if (demoWindow) ImGui::ShowDemoWindow(&demoWindow);

        Window&     window = m_App->GetWindow();
        static bool vsync  = false;
        if (ImGui::Checkbox("VSync", &vsync))
            window.GetSwapChain()->SetVSync(vsync);

        static f32 fov         = 75.0f;
        static f32 aspectRatio = static_cast<f32>(window.GetWidth())
                               / static_cast<f32>(window.GetHeight());
        static f32 planes[] = {0.1f, 100.0f};

        if (ImGui::DragFloat("fov", &fov, 0.1, 45.0f, 145.0f)
            || ImGui::DragFloat2("Near/Far planes", planes, 0.1f, 0.1f))
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
