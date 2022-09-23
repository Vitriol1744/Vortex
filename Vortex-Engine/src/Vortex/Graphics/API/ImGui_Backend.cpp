//
// Created by Vitriol1744 on 21.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "ImGui_Backend.hpp"

#include "Vortex/Graphics/API/imgui_impl_glfw.h"
#include "Vortex/Graphics/API/Vulkan/imgui_impl_vulkan.h"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanGraphicsPipeline.hpp"

namespace Vortex::Graphics::ImGui
{
    void Initialize(Ref<IWindow> mainWindow)
    {
        ::ImGui::CreateContext();
        ::ImGui::StyleColorsDark();

        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        static VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
        poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
        poolInfo.pPoolSizes = poolSizes;
        vkCreateDescriptorPool(device, &poolInfo, VulkanAllocator::Get().callbacks, &descriptorPool);

        ImGuiIO& io = ::ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui_ImplGlfw_InitForVulkan(reinterpret_cast<GLFWwindow*>(mainWindow->GetNativeHandle()), true);
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = VulkanRendererAPI::GetVulkanInstance().instance;
        initInfo.PhysicalDevice = VulkanRendererAPI::GetDevice().GetPhysicalDevice();
        initInfo.Device = device;
        initInfo.QueueFamily = VulkanRendererAPI::GetDevice().GetGraphicsQueueFamily();
        initInfo.Queue = VulkanRendererAPI::GetDevice().GetGraphicsQueue();
        initInfo.DescriptorPool = descriptorPool;
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = 2;
        initInfo.Allocator = VulkanAllocator::Get().callbacks;
        VkRenderPass renderPass = VulkanGraphicsPipeline::GetBound()->GetRenderPass();
        ImGui_ImplVulkan_Init(&initInfo, renderPass);

        VkCommandBuffer commandBuffer = VulkanRendererAPI::GetCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(initInfo.Queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(initInfo.Queue);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
    void Shutdown()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    void Begin()
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ::ImGui::NewFrame();
    }
    void End()
    {
        ImGuiIO& io = ::ImGui::GetIO();
        ::ImGui::Render();
        VkCommandBuffer commandBuffer = VulkanRendererAPI::GetCommandBuffer();
        VkPipeline pipeline = VulkanGraphicsPipeline::GetBound()->GetPipeline();
        ImGui_ImplVulkan_RenderDrawData(::ImGui::GetDrawData(), commandBuffer);
    }
}
