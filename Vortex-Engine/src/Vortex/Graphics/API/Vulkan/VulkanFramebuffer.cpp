//
// Created by Vitriol1744 on 15.09.2022.
//
#include "VulkanFramebuffer.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"

namespace Vortex::Graphics
{
    VulkanFramebuffer::VulkanFramebuffer(Ref<IGraphicsPipeline> pipeline)
    {
        Ref<VulkanGraphicsPipeline> vulkanPipeline = std::dynamic_pointer_cast<VulkanGraphicsPipeline>(pipeline);
        VkRenderPass renderPass = vulkanPipeline->GetRenderPass();
        context = vulkanPipeline->GetContext();
        context->AttachFramebuffer(this, renderPass);
        context->SetFramebuffer(this);
    }
    VulkanFramebuffer::~VulkanFramebuffer()
    {
        context->DetachFramebuffer(this);
    }

    void VulkanFramebuffer::Initialize(std::vector<VkImageView> imageViews, VkRenderPass renderPass, VkExtent2D extent)
    {
        framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); i++)
        {
            VkImageView attachments[] = { imageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = extent.width;
            framebufferInfo.height = extent.height;
            framebufferInfo.layers = 1;

            VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
            VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
            VkCall_msg(vkCreateFramebuffer(device, &framebufferInfo, allocator, &framebuffers[i]),
                       "Failed to create the framebuffer!")
        }
    }
    void VulkanFramebuffer::Destroy()
    {
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
        for (auto framebuffer : framebuffers) vkDestroyFramebuffer(device, framebuffer, allocator);
    }

    void VulkanFramebuffer::Bind()
    {
        context->SetFramebuffer(this);
    }
}
