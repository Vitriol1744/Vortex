/*
 * Created by v1tr10l7 on 27.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "SandboxLayer2D.hpp"

#include <imgui.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Vortex/Core/Log/Log.hpp"
#include "Vortex/Core/Math/Matrix.hpp"
#include "Vortex/Core/Profiler.hpp"
#include "Vortex/Core/Time.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Texture2D.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"
#include "Vortex/Renderer/Renderer.hpp"
#include "Vortex/Renderer/Renderer2D.hpp"
#include "Vortex/Window/Input/Keyboard.hpp"

#include "ImGuiPanels.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Vortex;
struct Vertex
{
    Vec3 Pos;
    Vec3 Color;
    Vec2 TexCoords;
};

static f32                s_MovementSpeed = 0.5f;
static glm::vec3          s_Translation   = glm::vec3(0.0f, 0.0f, 0.0f);

static f32                s_ZoomLevel     = 3.0f;

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys,
    lastTotalIdle;

struct UniformBufferObject
{
    alignas(16) Mat4 Model;
    alignas(16) Mat4 ViewProjection;
};

static Ref<Shader>           s_Shader           = nullptr;
static Ref<GraphicsPipeline> s_GraphicsPipeline = nullptr;
static Ref<VertexBuffer>     s_VertexBuffer     = nullptr;
static Ref<IndexBuffer>      s_IndexBuffer      = nullptr;
static Ref<UniformBuffer>    s_UniformBuffer    = nullptr;
static Ref<Texture2D>        s_Texture2D        = nullptr;

static Camera                s_Camera;

using namespace Vortex;

static std::vector<Vertex> vertices;
static std::vector<u32>    indices;

void                       ProcessMesh(aiMesh* mesh, const aiScene* scene)
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
        ProcessMesh(mesh, scene);
    }
    for (usize i = 0; i < node->mNumChildren; i++)
        ProcessNode(node->mChildren[i], scene);
}

void SandboxLayer2D::OnAttach()
{
    VtProfileFunction();
    auto& window = Application::Get()->GetWindow();
    window.SetPosition({300, 300});

    Image image("assets/icon.bmp");
    s_Shader = Shader::Create("assets/shaders/texture.glsl");

    window.SetIcon(image);
    window.ShowCursor();

    std::initializer_list<VertexBufferElement> elements
        = {ShaderDataType::eFloat3, ShaderDataType::eFloat3,
           ShaderDataType::eFloat2};
    VertexBufferLayout layout(elements);
    Assimp::Importer   importer;
    const aiScene*     scene
        = importer.ReadFile("assets/models/viking_room.obj",
                            aiProcess_Triangulate | aiProcess_FlipUVs);
    ProcessNode(scene->mRootNode, scene);
    importer.FreeScene();

    // GraphicsPipelineSpecification specification{};
    // specification.Window = Application::Get()->GetWindow();
    // specification.Shader = s_Shader;
    // specification.Layout = layout;

    // s_GraphicsPipeline   = GraphicsPipeline::Create(specification);
    // s_VertexBuffer       = VertexBuffer::Create(
    //(void*)vertices.data(), vertices.size() * sizeof(vertices[0]));
    // s_IndexBuffer
    //   = IndexBuffer::Create(indices.data(), indices.size() * sizeof(u32));
    // s_UniformBuffer   = UniformBuffer::Create(sizeof(UniformBufferObject));

    // s_Texture2D       = Texture2D::Create("assets/textures/viking_room.png");

    // vk::Device device = VulkanContext::GetDevice();
    VtCoreInfo("Frames in flight: {}",
               Renderer::GetConfiguration().MaxFramesInFlight);

    // s_Shader->SetUniform("UniformBufferObject", s_UniformBuffer);
    // s_Shader->SetUniform("texSampler", s_Texture2D);

    f32 aspectRatio = static_cast<f32>(window.GetWidth())
                    / static_cast<f32>(window.GetHeight());
    f32 zoomLevel = 3.0f;

    s_Camera.SetOrthographic(aspectRatio * zoomLevel, -aspectRatio * zoomLevel,
                             zoomLevel, -zoomLevel, 0.0f, 1.0f);

    Vec2        mousePos        = window.GetCursorPosition();

    static auto onMouseScrolled = [](Window*, f64, f64 deltaY) -> bool
    {
        s_ZoomLevel += deltaY * Application::Get()->GetDeltaTime();
        return false;
    };

    WindowEvents::MouseScrolledEvent += onMouseScrolled;
}
void SandboxLayer2D::OnDetach() { s_Shader.reset(); }

void SandboxLayer2D::OnUpdate()
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
    if (Keyboard::IsKeyPressed(KeyCode::eA))
        s_Translation.x += s_MovementSpeed * Application::Get()->GetDeltaTime();
    else if (Keyboard::IsKeyPressed(KeyCode::eD))
        s_Translation.x -= s_MovementSpeed * Application::Get()->GetDeltaTime();
    else if (Keyboard::IsKeyPressed(KeyCode::eW))
        s_Translation.y -= s_MovementSpeed * Application::Get()->GetDeltaTime();
    else if (Keyboard::IsKeyPressed(KeyCode::eS))
        s_Translation.y += s_MovementSpeed * Application::Get()->GetDeltaTime();

    s_Camera.SetPosition(s_Translation);
}
void SandboxLayer2D::OnRender()
{
    VtProfileFunction();
    // Renderer::Draw(s_GraphicsPipeline, s_VertexBuffer, s_IndexBuffer);

    Renderer2D::BeginScene(s_Camera);
    Renderer2D::DrawQuad(glm::vec2(-1.0f, 0.0f), glm::vec2(0.8f, 0.8f),
                         Vec4(1.0f, 0.0f, 1.0f, 1.0f));
    Renderer2D::DrawQuad(glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.75f),
                         Vec4(0.0f, 1.0f, 1.0f, 1.0f));
    Renderer2D::DrawQuad(glm::vec2(1.5f, -0.5f), glm::vec2(0.2f, 0.45f),
                         Vec4(0.0f, 1.0f, 1.0f, 1.0f));

    Vec4 redColor(1.0f, 0.0f, 0.0f, 1.0f);
    Vec4 blueColor(0.0f, 0.0f, 1.0f, 1.0f);

    Vec3 scale(0.05f, 0.05f, 0.5f);

    for (f32 y = -5.0f; y < 5.0f; y += 0.5f)
    {
        for (f32 x = -5.0f; x < 5.0f; x += 0.5f)
        {
            Vec2 pos(x * 0.11f, y * 0.11f);
            // Mat4 transform = glm::translate(Mat4(1.0f), pos) * scale;
            Vec4 color
                = Vec4((x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.2f);
            // if (x % 2 == 0) color = blueColor;
            Renderer2D::DrawQuad(pos, scale, color);
        }
    }
}
void SandboxLayer2D::OnImGuiRender()
{
    VtProfileFunction();
    auto& window = Application::Get()->GetWindow();
    auto  swapChain
        = std::dynamic_pointer_cast<VulkanSwapChain>(window.GetSwapChain());
    vk::Extent2D extent     = swapChain->GetExtent();

    bool         showWindow = true;
    ImGui::ShowDemoWindow(&showWindow);
    ImGui::Text("FPS: %lu", Application::Get()->GetFPSCounter());
    ImGui::Text("Delta Time: %f", Application::Get()->GetDeltaTime());

    std::ifstream ifs("/proc/stat");
    std::string   cpuUsage{};
    std::getline(ifs, cpuUsage);

    ImGui::Text("CPU: %s", cpuUsage.c_str());
    ImGui::Text("VRAM: %luMB/%luMB", Renderer::GetMemoryUsage() / 1024 / 1024,
                Renderer::GetMemoryBudget() / 1024 / 1024);
    ImGui::Text("Memory Budget: %luMB",
                Renderer::GetMemoryBudget() / 1024 / 1024);
    ImGui::SliderFloat("Zoom Level", &s_ZoomLevel, 0.0f, 10.0f);

    ImGuiPanels::DrawWindowOptions(window);

    ImGui::SliderFloat3("View", reinterpret_cast<float*>(&s_Translation), -10,
                        10);
    ImGui::SliderFloat("MovementSpeed", &s_MovementSpeed, 0.1f, 10.0f);

    static auto         startTime = Time::GetCurrentTime();
    f32                 time      = Time::GetCurrentTime() - startTime;

    UniformBufferObject ubo{};
    ubo.Model          = glm::rotate(Mat4(1.0f), time * glm::radians(90.0f),
                                     Vec3(0.0f, 0.0f, 1.0f));
    /*Mat4 view = glm::lookAt(Vec3(2.0f, 2.0f, 2.0f), Vec3(0.0f, 0.0f, 0.0f),
                            Vec3(0.0f, 0.0f, 1.0f));

    Mat4 projection      = glm::perspective(
        glm::radians(45.0f),
        extent.width / (f32)extent.height, 0.1f, 10.0f);*/

    ubo.ViewProjection = s_Camera.GetViewProjection();

    // s_UniformBuffer->SetData(&ubo, sizeof(ubo), 0);

    Renderer2D::EndScene();
}
