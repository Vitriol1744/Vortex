//
// Created by Vitriol1744 on 08.09.2022.
//
#include "VulkanRendererAPI.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanVertexBuffer.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanIndexBuffer.hpp"

namespace Vortex::Graphics
{
    VulkanRendererAPI* VulkanRendererAPI::vkRendererAPI_Instance = nullptr;

    void VulkanRendererAPI::InitializeImpl()
    {
        vkRendererAPI_Instance = dynamic_cast<VulkanRendererAPI*>(instance);
        VT_CORE_ASSERT(vkRendererAPI_Instance != nullptr);

        uint32 appVersionMajor = static_cast<uint32>(applicationVersion.x);
        uint32 appVersionMinor = static_cast<uint32>(applicationVersion.y);
        uint32 appVersionPatch = static_cast<uint32>(applicationVersion.z);

        vulkanInstance.Create(applicationName, appVersionMajor, appVersionMinor, appVersionPatch, UseValidationLayers());
    }

    void VulkanRendererAPI::BeginRenderPassImpl()
    {
    begin:
        currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

        VkDevice vkDevice = device.GetLogicalDevice();
        vkWaitForFences(vkDevice, 1, &inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);

        VulkanGraphicsPipeline* pipeline = VulkanGraphicsPipeline::GetBound();
        VulkanContext* context = pipeline->GetContext();
        VulkanSwapChain& swapChain = context->GetSwapChain();

        uint32_t& imageIndex = swapChain.GetImageIndex();
        VkResult result = vkAcquireNextImageKHR(vkDevice, swapChain.GetSwapChain(), UINT64_MAX, imageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            context->RecreateSwapChain();
            goto begin;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            VTCoreLogFatal("Vulkan: Failed to aquire the swapchain image! VkResult: {}", TranslateVkResult(result));
            return;
        }
        vkResetFences(vkDevice, 1, &inFlightFences[currentFrameIndex]);

        vkResetCommandBuffer(commandBuffers[currentFrameIndex], 0);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkCall_msg(vkBeginCommandBuffer(commandBuffers[currentFrameIndex], &beginInfo),
                   "Failed to begin recording the command buffer!");

        VulkanFramebuffer* framebuffer = context->GetBoundFramebuffer();
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pipeline->GetRenderPass();
        renderPassInfo.framebuffer = framebuffer->GetFramebuffers()[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChain.GetExtent();

        float32 r = clearColor.r;
        float32 g = clearColor.g;
        float32 b = clearColor.b;
        float32 a = clearColor.a;
        VkClearValue vkClearColor = {{{r, g, b, a}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &vkClearColor;

        vkCmdBeginRenderPass(commandBuffers[currentFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChain.GetExtent().width;
        viewport.height = (float) swapChain.GetExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrameIndex], 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = swapChain.GetExtent();
        vkCmdSetScissor(commandBuffers[currentFrameIndex], 0, 1, &scissor);
    }
    void VulkanRendererAPI::EndRenderPassImpl()
    {
        vkCmdEndRenderPass(commandBuffers[currentFrameIndex]);

        VkCall_msg(vkEndCommandBuffer(commandBuffers[currentFrameIndex]), "Failed to record command buffer!");

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrameIndex] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrameIndex];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrameIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkCall_msg(vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrameIndex]),
                   "Failed to submit draw command buffer!")
    }

    void VulkanRendererAPI::DrawImpl(Ref<IVertexBuffer> vertexBuffer)
    {
        Ref<VulkanVertexBuffer> buffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(vertexBuffer);

        VkBuffer vertexBuffers[] = { buffer->GetVulkanBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffers[currentFrameIndex], 0, 1, vertexBuffers, offsets);
        vkCmdDraw(commandBuffers[currentFrameIndex], vertexBuffer->GetSize(), 1, 0, 0);
    }
    void VulkanRendererAPI::DrawIndexedImpl(Ref<IVertexBuffer> vertexBuffer, Ref<IIndexBuffer> indexBuffer, uint16 indexCount)
    {
        Ref<VulkanVertexBuffer> vbuffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(vertexBuffer);
        Ref<VulkanIndexBuffer> ibuffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(indexBuffer);

        VkBuffer vertexBuffers[] = { vbuffer->GetVulkanBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffers[currentFrameIndex], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffers[currentFrameIndex], ibuffer->GetVulkanBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffers[currentFrameIndex], indexCount, 1, 0, 0, 0);
    }

    void VulkanRendererAPI::SetClearColorImpl(Color color)
    {
        clearColor = color;
    }
    void VulkanRendererAPI::SetViewportImpl(const Math::Rectangle& rect)
    {
        //TODO: Vulkan: SetViewportImpl!
    }

    void VulkanRendererAPI::ClearImpl() noexcept
    {
        //TODO: Vulkan: ClearImpl!
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRendererAPI::ErrorCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                    VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* userData)
    {
        switch (severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                VTCoreLogTrace("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                VTCoreLogInfo("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                VTCoreLogWarn("Vulkan Validation Layer: {}", data->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                VTCoreLogError("Vulkan Validation Layer: {}", data->pMessage);
                break;

            default:
                break;
        }

        return VK_FALSE;
    }

    void VulkanRendererAPI::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = device.GetGraphicsQueueFamily();

        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
        VkCall_msg(vkCreateCommandPool(device.GetLogicalDevice(), &poolInfo, allocator, &commandPool),
                   "Failed to create command pool!");
    }
    void VulkanRendererAPI::DestroyCommandPool()
    {
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
        vkDestroyCommandPool(device.GetLogicalDevice(), commandPool, allocator);
    }
    void VulkanRendererAPI::CreateCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = commandBuffers.size();

        VkCall_msg(vkAllocateCommandBuffers(device.GetLogicalDevice(), &allocInfo, commandBuffers.data()),
                "Failed to create command buffers!");
    }
    void VulkanRendererAPI::CreateSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
        for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkCall_msg(vkCreateSemaphore(device.GetLogicalDevice(), &semaphoreInfo, allocator, &imageAvailableSemaphores[i]),
                       "Failed to create the semaphore!");
            VkCall_msg(vkCreateSemaphore(device.GetLogicalDevice(), &semaphoreInfo, allocator, &renderFinishedSemaphores[i]),
                       "Failed to create the semaphore!");
            VkCall_msg(vkCreateFence(device.GetLogicalDevice(), &fenceInfo, allocator, &inFlightFences[i]),
                       "Failed to create the fence object!");
        }
    }
    void VulkanRendererAPI::DestroySyncObjects()
    {
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;

        for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device.GetLogicalDevice(), renderFinishedSemaphores[i], allocator);
            vkDestroySemaphore(device.GetLogicalDevice(), imageAvailableSemaphores[i], allocator);
            vkDestroyFence(device.GetLogicalDevice(), inFlightFences[i], allocator);
        }
    }
}