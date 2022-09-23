//
// Created by Vitriol1744 on 14.09.2022.
//
#include "VulkanContext.hpp"

namespace Vortex::Graphics
{
    VulkanContext::VulkanContext(uintptr_t _windowHandle, uint32 windowWidth, uint32 windowHeight)
    {
        windowHandle = reinterpret_cast<GLFWwindow*>(_windowHandle);
        surface.Create(windowHandle);
        swapChain.Create(surface, windowWidth, windowHeight);

        EventSystem::GetInstance()->SubscribeEvent(FramebufferResizedEvent::GetStaticEventID(), this);
    }
    VulkanContext::~VulkanContext()
    {
        swapChain.Destroy();
        surface.Destroy();
    }

    void VulkanContext::Present()
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSemaphore signalSemaphores[] = { VulkanRendererAPI::GetRenderFinishedSemaphore() };
        uint32_t& imageIndex = swapChain.GetImageIndex();

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain.GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        VkQueue presentQueue = VulkanRendererAPI::GetDevice().GetPresentQueue();
        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) RecreateSwapChain();
        else if (result != VK_SUCCESS)
        {
            VTCoreLogFatal("Vulkan: Failed to aquire the swapchain image! VkResult: {}", TranslateVkResult(result));
            return;
        }
    }
    bool VulkanContext::OnEvent(IEvent& e)
    {
        auto onResize = [this](IEvent& e)
        {
            this->RecreateSwapChain();
        };
        DispatchEvent<FramebufferResizedEvent>(e, onResize);

        return false;
    }

    void VulkanContext::SetGraphicsPipeline(VulkanGraphicsPipeline* pipeline)
    {
        boundGraphicsPipeline = pipeline;
    }
    void VulkanContext::SetFramebuffer(VulkanFramebuffer* framebuffer)
    {
        boundFramebuffer = framebuffer;
    }
    void VulkanContext::AttachGraphicsPipeline(VulkanGraphicsPipeline* pipeline, GraphicsPipelineCreateInfo& createInfo)
    {
        graphicsPipelines.push_back(pipeline);
        VkExtent2D extent = swapChain.GetExtent();
        VkFormat format = swapChain.GetImageFormat();
        pipeline->Initialize(createInfo, extent, format);
    }
    void VulkanContext::DetachGraphicsPipeline(VulkanGraphicsPipeline* pipeline)
    {
        std::remove_if(graphicsPipelines.begin(), graphicsPipelines.end(), [pipeline](VulkanGraphicsPipeline* it)
        {
            if (it == pipeline)
            {
                pipeline->Destroy();
                return true;
            }
            return false;
        });
    }
    void VulkanContext::AttachFramebuffer(VulkanFramebuffer* framebuffer, VkRenderPass renderPass)
    {
        framebuffers.push_back(framebuffer);
        std::vector<VkImageView>& imageViews = swapChain.GetImageViews();
        VkExtent2D extent = swapChain.GetExtent();

        framebuffer->Initialize(imageViews, renderPass, extent);
    }
    void VulkanContext::DetachFramebuffer(VulkanFramebuffer* framebuffer)
    {
        std::remove_if(framebuffers.begin(), framebuffers.end(), [framebuffer](VulkanFramebuffer* it)
        {
            if (it == framebuffer)
            {
                framebuffer->Destroy();
                return true;
            }
            return false;
        });
    }

    void VulkanContext::RecreateSwapChain()
    {
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        vkDeviceWaitIdle(device);

        boundFramebuffer->Destroy();
        swapChain.Destroy();
        int32 width, height;
        glfwGetFramebufferSize(windowHandle, &width, &height);

        swapChain.Create(surface, width, height);
        std::vector<VkImageView>& imageViews = swapChain.GetImageViews();
        VkRenderPass renderPass = boundGraphicsPipeline->GetRenderPass();
        VkExtent2D extent = swapChain.GetExtent();
        boundFramebuffer->Initialize(imageViews, renderPass, extent);
    }
}