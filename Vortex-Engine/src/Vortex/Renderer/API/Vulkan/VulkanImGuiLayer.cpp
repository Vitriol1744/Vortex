/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Engine/Application.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImGuiLayer.hpp"
#include "Vortex/Renderer/API/Vulkan/imgui_impl_vulkan.h"
#include "Vortex/Renderer/Window/imgui_impl_glfw.h"

namespace Vortex
{
    void VulkanImGuiLayer::OnAttach()
    {

        auto window  = Application::Get()->GetWindow();
        auto context = std::dynamic_pointer_cast<VulkanContext>(
            window->GetRendererContext());
        vk::Queue graphicsQueue = VulkanContext::GetDevice().GetGraphicsQueue();

        vk::DescriptorPoolSize poolSizes[] = {
            {vk::DescriptorType::eCombinedImageSampler, 1},
        };

        vk::DescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
        poolInfo.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
        poolInfo.pPoolSizes    = poolSizes;

        vk::Device device      = VulkanContext::GetDevice();
        VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                           &m_DescriptorPool));

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags
            |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags
            |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags
            |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
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

        auto indices
            = VulkanContext::GetPhysicalDevice().GetQueueFamilyIndices();
        auto glfwWindow = std::any_cast<GLFWwindow*>(window->GetNativeHandle());

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = vk::Instance(VulkanContext::GetInstance());
        initInfo.PhysicalDevice
            = vk::PhysicalDevice(VulkanContext::GetPhysicalDevice());
        initInfo.Device         = device;
        initInfo.QueueFamily    = indices.Graphics.value();
        initInfo.Queue          = graphicsQueue;
        initInfo.PipelineCache  = VK_NULL_HANDLE;
        initInfo.DescriptorPool = m_DescriptorPool;
        initInfo.RenderPass     = context->GetSwapChain().GetRenderPass();
        initInfo.Subpass        = 0;
        u32 frameCount        = static_cast<u32>(context->GetSwapChain().GetFrames().size());
        initInfo.MinImageCount  = frameCount;
        initInfo.ImageCount     = frameCount;
        initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator      = VK_NULL_HANDLE;
        initInfo.CheckVkResultFn
            = [](VkResult result) -> void { VkCall(vk::Result(result)); };
        ImGui_ImplVulkan_Init(&initInfo);
    }
    void VulkanImGuiLayer::OnDetach()
    {

        vk::Device device = VulkanContext::GetDevice();
        device.waitIdle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(device, m_DescriptorPool, VK_NULL_HANDLE);
    }

    void VulkanImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void VulkanImGuiLayer::End()
    {
        // ImGui::Render();
        ImGuiIO io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void VulkanImGuiLayer::OnUpdate() {}
    void VulkanImGuiLayer::OnRender() {}
    void VulkanImGuiLayer::OnImGuiRender() {}

}; // namespace Vortex
