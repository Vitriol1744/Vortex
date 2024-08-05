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
#include "Vortex/Engine/Application.hpp"
#include "Vortex/Renderer/API/Shader.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanIndexBuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanVertexBuffer.hpp"
#include "Vortex/Utility/ImageLoader.hpp"

using namespace Vortex;
struct Vertex
{
    Vec2                                     Pos;
    Vec3                                     Color;

    static vk::VertexInputBindingDescription GetBindingDescription()
    {
        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding   = 0;
        bindingDescription.stride    = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }
    static std::array<vk::VertexInputAttributeDescription, 2>
    GetAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 2>
            attributeDescriptions{};
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset   = offsetof(Vertex, Pos);
        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset   = offsetof(Vertex, Color);

        return attributeDescriptions;
    }
};
const std::vector<Vertex> s_Vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
const std::vector<u16>    indices    = {0, 1, 2, 2, 3, 0};

static Ref<Window>        s_Window   = nullptr;
static Ref<VulkanContext> s_Context  = nullptr;
static Ref<Shader>        s_Shader   = nullptr;
static Ref<VulkanGraphicsPipeline> s_GraphicsPipeline = nullptr;
static Ref<VulkanVertexBuffer>     s_VertexBuffer     = nullptr;
static Ref<VulkanIndexBuffer>      s_IndexBuffer      = nullptr;
static Scope<Pixel[]>              pixels             = nullptr;

using namespace Vortex;

void SandboxLayer2D::OnAttach()
{
    s_Window = Application::Get()->GetWindow();
    s_Window->SetPosition({300, 300});
    s_Context = std::dynamic_pointer_cast<VulkanContext>(
        s_Window->GetRendererContext());

    s_Shader = Shader::Create("vert2.spv", "frag.spv");
    i32  width, height;
    auto ret = ImageLoader::LoadBMP("assets/icon.bmp", width, height);
    if (!ret) { VtError("error: {}", ret.error()); }
    else pixels = std::move(ret.value());
    Image img;
    img.Pixels = pixels.get();
    img.Width  = width;
    img.Height = height;

    // s_Window->SetIcon(img);

    std::initializer_list<VertexBufferElement> elements
        = {ShaderDataType::eFloat2, ShaderDataType::eFloat3};
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
        (void*)indices.data(), indices.size() * sizeof(uint16_t));
}
void SandboxLayer2D::OnDetach() {}

void SandboxLayer2D::OnUpdate() {}
void SandboxLayer2D::OnRender() {}
void SandboxLayer2D::OnImGuiRender()
{
    bool showWindow = true;
    ImGui::ShowDemoWindow(&showWindow);
    ImGui::Render();

    VulkanSwapChain& swapChain = s_Context->GetSwapChain();
    swapChain.BeginFrame();

    vk::Extent2D      extent        = swapChain.GetExtent();
    vk::CommandBuffer commandBuffer = swapChain.GetCurrentCommandBuffer();

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *s_GraphicsPipeline);

    vk::Viewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<f32>(extent.width);
    viewport.height   = static_cast<f32>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = extent;
    commandBuffer.setScissor(0, 1, &scissor);
    vk::Buffer     vertexBuffers[] = {s_VertexBuffer->GetBuffer()};
    vk::DeviceSize offsets[]       = {0};
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(s_IndexBuffer->GetBuffer(), 0,
                                  vk::IndexType::eUint16);

    // commandBuffer.draw(s_Vertices.size(), 1, 0, 0);
    commandBuffer.drawIndexed(u32(indices.size()), 1, 0, 0, 0);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    swapChain.EndFrame();
}
