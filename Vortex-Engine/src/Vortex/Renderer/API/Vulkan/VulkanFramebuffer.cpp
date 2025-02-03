/*
 * Created by v1tr10l7 on 09.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanFramebuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/Renderer.hpp"

namespace Vortex
{
    VulkanFramebuffer::VulkanFramebuffer(
        const vk::Extent2D& extent, const std::vector<vk::Image>& backbuffers,
        const vk::Format& format)
    {
        m_ImageFormat = format;
        m_Extent      = extent;
        m_Frames.resize(backbuffers.size());
        for (u32 i = 0; i < backbuffers.size(); i++)
            m_Frames[i].Image = backbuffers[i];

        CreateImageViews();
        CreateDepthBuffer();
        CreateRenderPass();
        CreateFramebuffers();
    }
    VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specs)
    {
        m_SwapChainTarget = specs.SwapChainTarget;
        m_ImageFormat     = vk::Format::eB8G8R8A8Srgb;
        m_Extent          = vk::Extent2D(specs.Width, specs.Height);
        m_Frames.resize(Renderer::GetConfiguration().MaxFramesInFlight);
        Create();
    }

    VulkanFramebuffer::~VulkanFramebuffer() { Destroy(); }

    void VulkanFramebuffer::Create()
    {
        CreateImages();
        CreateImageViews();
        CreateDepthBuffer();
        CreateRenderPass();
        CreateFramebuffers();
        CreateSamplers();
    }
    void VulkanFramebuffer::Destroy()
    {

        vk::Device device = VulkanRenderer::GetDevice();
        device.waitIdle();

        device.destroyImageView(m_DepthImageView);
        m_DepthImage.Destroy();

        for (u32 i = 0; i < m_Frames.size(); i++)
        {
            device.destroyFramebuffer(m_Frames[i].Framebuffer, VK_NULL_HANDLE);
            device.destroyImageView(m_Frames[i].ImageView, VK_NULL_HANDLE);
            if (!m_SwapChainTarget)
            {
                VulkanAllocator::DestroyImage(m_Frames[i].Image,
                                              m_ImageAllocations[i]);
                device.destroySampler(m_Frames[i].Sampler);
            }
        }
        device.destroyRenderPass(m_RenderPass);
    }

    void VulkanFramebuffer::OnResize(u32 width, u32 height)
    {
        vk::Device vkDevice = VulkanRenderer::GetDevice();
        vkDevice.waitIdle();

        Destroy();
        m_Extent = vk::Extent2D(width, height);
        Create();

        vkDevice.waitIdle();
    }
    void VulkanFramebuffer::NextImage()
    {
        u32 framesInFlight  = Renderer::GetConfiguration().MaxFramesInFlight;
        m_CurrentImageIndex = (m_CurrentImageIndex + 1) % framesInFlight;
    }

    void VulkanFramebuffer::CreateImages()
    {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.sType         = vk::StructureType::eImageCreateInfo;
        imageInfo.pNext         = VK_NULL_HANDLE;
        imageInfo.flags         = vk::ImageCreateFlags();
        imageInfo.imageType     = vk::ImageType::e2D;
        imageInfo.format        = m_ImageFormat;
        imageInfo.extent.width  = m_Extent.width;
        imageInfo.extent.height = m_Extent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.samples       = vk::SampleCountFlagBits::e1;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.usage         = vk::ImageUsageFlagBits::eColorAttachment
                        | vk::ImageUsageFlagBits::eSampled;
        imageInfo.sharingMode           = vk::SharingMode::eExclusive;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;
        imageInfo.initialLayout         = vk::ImageLayout::eUndefined;

        vk::DeviceSize size             = m_Extent.width * m_Extent.height * 4;

        m_ImageAllocations.resize(m_Frames.size());
        for (u32 i = 0; i < m_Frames.size(); i++)
            m_ImageAllocations[i] = VulkanAllocator::AllocateImage(
                imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Frames[i].Image, size);
    }
    void VulkanFramebuffer::CreateImageViews()
    {
        vk::Device device = VulkanRenderer::GetDevice();

        for (size_t i = 0; i < m_Frames.size(); i++)
        {
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.sType        = vk::StructureType::eImageViewCreateInfo;
            viewInfo.pNext        = VK_NULL_HANDLE;
            viewInfo.flags        = vk::ImageViewCreateFlags();
            viewInfo.image        = m_Frames[i].Image;
            viewInfo.viewType     = vk::ImageViewType::e2D;
            viewInfo.format       = m_ImageFormat;
            viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
            viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
            viewInfo.subresourceRange.aspectMask
                = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel   = 0;
            viewInfo.subresourceRange.levelCount     = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount     = 1;

            VkCall(device.createImageView(&viewInfo, VK_NULL_HANDLE,
                                          &m_Frames[i].ImageView));
        }
    }

    void VulkanFramebuffer::CreateDepthBuffer()
    {
        vk::Format depthFormat
            = VulkanRenderer::GetPhysicalDevice().FindDepthFormat();

        vk::Device device = VulkanRenderer::GetDevice();
        if (m_DepthImage)
        {
            device.destroyImageView(m_DepthImageView);
            m_DepthImage.Destroy();
        }
        m_DepthImage.Create(m_Extent.width, m_Extent.height, depthFormat,
                            vk::ImageTiling::eOptimal,
                            vk::ImageUsageFlagBits::eDepthStencilAttachment);
        m_DepthImage.TransitionLayout(
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.sType        = vk::StructureType::eImageViewCreateInfo;
        viewInfo.pNext        = VK_NULL_HANDLE;
        viewInfo.flags        = vk::ImageViewCreateFlags();
        viewInfo.image        = m_DepthImage;
        viewInfo.viewType     = vk::ImageViewType::e2D;
        viewInfo.format       = depthFormat;
        viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.a = vk::ComponentSwizzle::eIdentity;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        VkCall(device.createImageView(&viewInfo, VK_NULL_HANDLE,
                                      &m_DepthImageView));
    }

    void VulkanFramebuffer::CreateRenderPass()
    {
        vk::Device                device = VulkanRenderer::GetDevice();

        vk::AttachmentDescription colorAttachment{};
        colorAttachment.flags          = vk::AttachmentDescriptionFlags();
        colorAttachment.format         = m_ImageFormat;
        colorAttachment.samples        = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout
            = m_SwapChainTarget ? vk::ImageLayout::ePresentSrcKHR
                                : vk::ImageLayout::eShaderReadOnlyOptimal;

        vk::Format depthFormat
            = VulkanRenderer::GetPhysicalDevice().FindDepthFormat();
        vk::AttachmentDescription depthAttachment{};
        depthAttachment.flags          = vk::AttachmentDescriptionFlags();
        depthAttachment.format         = depthFormat;
        depthAttachment.samples        = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp        = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout
            = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout     = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference depthReference{};
        depthReference.attachment = 1;
        depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpassDescription{};
        subpassDescription.flags             = vk::SubpassDescriptionFlags();
        subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpassDescription.inputAttachmentCount    = 0;
        subpassDescription.pInputAttachments       = VK_NULL_HANDLE;
        subpassDescription.colorAttachmentCount    = 1;
        subpassDescription.pColorAttachments       = &colorReference;
        subpassDescription.pResolveAttachments     = VK_NULL_HANDLE;
        subpassDescription.pDepthStencilAttachment = &depthReference;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments    = VK_NULL_HANDLE;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput
            | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.dstStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput
            | vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcAccessMask = vk::AccessFlags();
        dependency.dstAccessMask
            = vk::AccessFlagBits::eColorAttachmentWrite
            | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependency.dependencyFlags = vk::DependencyFlags();

        std::array<vk::AttachmentDescription, 2> attachments
            = {colorAttachment, depthAttachment};

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.pNext = VK_NULL_HANDLE;
        renderPassInfo.flags = vk::RenderPassCreateFlags();
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpassDescription;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;

        VkCall(device.createRenderPass(&renderPassInfo, VK_NULL_HANDLE,
                                       &m_RenderPass));
    }
    void VulkanFramebuffer::CreateFramebuffers()
    {
        vk::Device device = VulkanRenderer::GetDevice();

        for (u32 i = 0; i < m_Frames.size(); i++)
        {
            std::array<vk::ImageView, 2> attachments
                = {m_Frames[i].ImageView, m_DepthImageView};

            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            framebufferInfo.pNext = VK_NULL_HANDLE;
            framebufferInfo.flags = vk::FramebufferCreateFlags();
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = 1;

            VkCall(device.createFramebuffer(&framebufferInfo, VK_NULL_HANDLE,
                                            &m_Frames[i].Framebuffer));
        }
    }
    void VulkanFramebuffer::CreateSamplers()
    {
        vk::Device            device = VulkanRenderer::GetDevice();

        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.sType            = vk::StructureType::eSamplerCreateInfo;
        samplerInfo.magFilter        = vk::Filter::eLinear;
        samplerInfo.minFilter        = vk::Filter::eLinear;
        samplerInfo.addressModeU     = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV     = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW     = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy    = 0;
        samplerInfo.borderColor      = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 0.0f;

        for (auto& frame : m_Frames)
            VkCall(device.createSampler(&samplerInfo, VK_NULL_HANDLE,
                                        &frame.Sampler));
    }
}; // namespace Vortex
