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

using namespace Vortex;

static Ref<Window>                 s_Window           = nullptr;
static Ref<VulkanContext>          s_Context          = nullptr;
static Ref<Shader>                 s_Shader           = nullptr;
static Ref<VulkanGraphicsPipeline> s_GraphicsPipeline = nullptr;

using namespace Vortex;

void SandboxLayer2D::OnAttach()
{
    s_Window  = Application::Get()->GetWindow();
    s_Window->SetPosition({300, 300});
    s_Context = std::dynamic_pointer_cast<VulkanContext>(
        s_Window->GetRendererContext());

    s_Shader                = Shader::Create();

    GraphicsPipelineSpecification specification{};
    specification.Window = Application::Get()->GetWindow();
    specification.Shader = s_Shader;

    s_GraphicsPipeline   = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(
        GraphicsPipeline::Create(specification));
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

    commandBuffer.draw(3, 1, 0, 0);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    swapChain.EndFrame();
}
