/*
 * Created by v1tr10l7 on 27.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "SandboxLayer2D.hpp"

#include "Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h"
#include <imgui.h>

#include "Vortex/Core/Log/Log.hpp"
#include "Vortex/Core/Math/Matrix.hpp"
#include "Vortex/Core/Timer.hpp"
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanIndexBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanUniformBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.hpp"
#include "Vortex/Renderer/Renderer.hpp"
#include "Vortex/Utility/ImageLoader.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

using namespace Vortex;
struct Vertex
{
    Vec3 Pos;
    Vec3 Color;
    Vec2 TexCoords;
};

const std::vector<Vertex> s_Vertices
    = {{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
       {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
       {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
       {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

       {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
       {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
       {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
       {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

// const std::vector<u32> indices = {0, 1, 2, 2, 3, 0};
const std::vector<u32> s_Indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

struct UniformBufferObject
{
    alignas(16) Mat4 Model;
    alignas(16) Mat4 View;
    alignas(16) Mat4 Projection;
    alignas(8) Vec2 lightPos;
};

static Ref<Window>                    s_Window           = nullptr;
static Ref<VulkanContext>             s_Context          = nullptr;
static Ref<VulkanShader>              s_Shader           = nullptr;
static Ref<VulkanGraphicsPipeline>    s_GraphicsPipeline = nullptr;
static Ref<VulkanVertexBuffer>        s_VertexBuffer     = nullptr;
static Ref<VulkanIndexBuffer>         s_IndexBuffer      = nullptr;
static Ref<VulkanUniformBuffer>       s_UniformBuffer    = nullptr;
static std::vector<vk::DescriptorSet> s_DescriptorSets;
static Scope<Pixel[]>                 pixels      = nullptr;
static Ref<VulkanTexture2D>           s_Texture2D = nullptr;
static bool                           s_VSync     = false;
Vec2                                  lightPos;

using namespace Vortex;

void SandboxLayer2D::OnAttach()
{
    s_Window = Application::Get()->GetWindow();
    s_Window->SetPosition({300, 300});
    s_Context = std::dynamic_pointer_cast<VulkanContext>(
        s_Window->GetRendererContext());

    Image image("assets/icon.bmp");
    s_Shader = CreateRef<VulkanShader>("assets/shaders/texture.glsl");

    s_Window->SetIcon(image);
    s_Window->ShowCursor();

    std::initializer_list<VertexBufferElement> elements
        = {ShaderDataType::eFloat3, ShaderDataType::eFloat3,
           ShaderDataType::eFloat2};
    VertexBufferLayout            layout(elements);

    GraphicsPipelineSpecification specification{};
    specification.Window = Application::Get()->GetWindow();
    specification.Shader = s_Shader;
    specification.Layout = layout;

    s_GraphicsPipeline   = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(
        GraphicsPipeline::Create(specification));
    s_VertexBuffer = CreateRef<VulkanVertexBuffer>(
        (void*)s_Vertices.data(), s_Vertices.size() * sizeof(s_Vertices[0]));
    s_IndexBuffer = CreateRef<VulkanIndexBuffer>(
        (void*)s_Indices.data(), s_Indices.size() * sizeof(u32));
    s_UniformBuffer
        = CreateRef<VulkanUniformBuffer>(sizeof(UniformBufferObject));

    s_Texture2D = CreateRef<VulkanTexture2D>("assets/textures/texture.jpg");

    vk::Device device = VulkanContext::GetDevice();
    VtCoreInfo("Frames in flight: {}", VT_MAX_FRAMES_IN_FLIGHT);

    s_DescriptorSets = s_Shader->GetDescriptorSets()[0].Sets;
    s_Shader->SetUniform("UniformBufferObject", s_UniformBuffer);
    s_Shader->SetUniform("texSampler", s_Texture2D);

    Vec2 mousePos = s_Window->GetCursorPosition();
    lightPos.x    = ((float)(mousePos.x * 16.0f / s_Window->GetWidth()));
    lightPos.y    = (float)(9.0f - mousePos.y * 9.0f / s_Window->GetHeight());
}
void SandboxLayer2D::OnDetach() {}

void SandboxLayer2D::OnUpdate() {}
void SandboxLayer2D::OnRender()
{
    Renderer::Draw(s_GraphicsPipeline, s_VertexBuffer, s_IndexBuffer);
}
void SandboxLayer2D::OnImGuiRender()
{
    VulkanSwapChain&  swapChain     = s_Context->GetSwapChain();

    vk::Extent2D      extent        = swapChain.GetExtent();
    vk::CommandBuffer commandBuffer = swapChain.GetCurrentCommandBuffer();
    Vec2              mousePos      = s_Window->GetCursorPosition();
    mousePos.x -= s_Window->GetWidth() / 2.f;
    mousePos.y -= s_Window->GetHeight() / 2.f;
    mousePos.x /= s_Window->GetWidth();
    mousePos.y /= s_Window->GetHeight();
    mousePos.x *= -1;

    bool showWindow = true;
    ImGui::ShowDemoWindow(&showWindow);
    ImGui::Text("FPS: %lu", Application::Get()->GetFPSCounter());
    ImGui::Text("Delta Time: %f", Application::Get()->GetDeltaTime());
    ImGui::Text("MousePos: { x: %f, y: %f }", mousePos.x, mousePos.y);

    ImGui::SliderFloat2("lightPos", (float*)&lightPos, -10, 10);
    if (ImGui::Button("Close")) Application::Get()->Close();
    if (ImGui::Button("Restart")) Application::Get()->Restart();
    ImGui::Checkbox("vsync", &s_VSync);
    swapChain.SetVSync(s_VSync);

    auto        currentFrame = swapChain.GetCurrentFrameIndex();
    static auto startTime    = std::chrono::high_resolution_clock::now();

    auto        currentTime  = std::chrono::high_resolution_clock::now();
    f32 time = std::chrono::duration<float, std::chrono::seconds::period>(
                   currentTime - startTime)
                   .count();

    UniformBufferObject ubo{};
    ubo.lightPos = mousePos;
    ubo.Model    = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.View
        = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                      glm::vec3(0.0f, 0.0f, 1.0f));

    auto swapChainExtent = s_Context->GetSwapChain().GetExtent();
    ubo.Projection       = glm::perspective(
        glm::radians(45.0f),
        swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

    ubo.Projection[1][1] *= -1;

    void* dest = VulkanAllocator::MapMemory(
        s_UniformBuffer->m_Allocations[currentFrame]);
    std::memcpy(dest, &ubo, sizeof(ubo));
    VulkanAllocator::UnmapMemory(s_UniformBuffer->m_Allocations[currentFrame]);
}
