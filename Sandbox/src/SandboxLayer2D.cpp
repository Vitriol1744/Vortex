/*
 * Created by v1tr10l7 on 27.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "SandboxLayer2D.hpp"

#include "Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h"
#include "Vortex/Renderer/Window/imgui_impl_glfw.h"
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

VkDescriptorPool                   descriptorPool     = VK_NULL_HANDLE;

using namespace Vortex;

void SandboxLayer2D::OnAttach()
{
    s_Window  = Application::Get()->GetWindow();
    s_Context = std::dynamic_pointer_cast<VulkanContext>(
        s_Window->GetRendererContext());
    vk::Queue graphicsQueue = VulkanContext::GetDevice().GetGraphicsQueue();

    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
    };
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = 1;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes    = poolSizes;

    vk::Device device      = VulkanContext::GetDevice();
    vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &descriptorPool);

    s_Shader = Shader::Create();

    GraphicsPipelineSpecification specification{};
    specification.Window = Application::Get()->GetWindow();
    specification.Shader = s_Shader;

    s_GraphicsPipeline   = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(
        GraphicsPipeline::Create(specification));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableGamepad;         // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows

    ImGui::StyleColorsDark();
    // When viewports are enabled we tweak WindowRounding/WindowBg so
    // platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    auto indices = VulkanContext::GetPhysicalDevice().GetQueueFamilyIndices();
    // Setup Platform/Renderer backends
    auto window  = std::any_cast<GLFWwindow*>(s_Window->GetNativeHandle());
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vk::Instance(VulkanContext::GetInstance());
    init_info.PhysicalDevice
        = vk::PhysicalDevice(VulkanContext::GetPhysicalDevice());
    init_info.Device          = device;
    init_info.QueueFamily     = indices.Graphics.value();
    init_info.Queue           = graphicsQueue;
    init_info.PipelineCache   = VK_NULL_HANDLE;
    init_info.DescriptorPool  = descriptorPool;
    init_info.RenderPass      = s_Context->GetSwapChain().GetRenderPass();
    init_info.Subpass         = 0;
    usize frameCount          = s_Context->GetSwapChain().GetFrames().size();
    init_info.MinImageCount   = frameCount;
    init_info.ImageCount      = frameCount;
    init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator       = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = [](VkResult result) -> void
    {
        if (result != VK_SUCCESS) std::cerr << "result != VK_SUCCESS";
    };
    ImGui_ImplVulkan_Init(&init_info);
}
void SandboxLayer2D::OnDetach()
{

    vk::Device device = VulkanContext::GetDevice();
    device.waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(device, descriptorPool, VK_NULL_HANDLE);
}

void SandboxLayer2D::OnUpdate() {}
void SandboxLayer2D::OnRender()
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

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
void SandboxLayer2D::OnImGuiRender()
{

    ImGuiIO io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}
