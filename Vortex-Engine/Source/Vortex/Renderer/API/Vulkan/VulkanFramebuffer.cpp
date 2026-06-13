/*
 * Created by v1tr10l7 on 09.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Array.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanFramebuffer.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/Renderer.hpp>

namespace Vortex
{
    VulkanFramebuffer::VulkanFramebuffer(const vk::Extent2D&      extent,
                                         const Vector<vk::Image>& backbuffers,
                                         const vk::Format&        format)
    {
        m_ImageFormat = format;
        m_Extent      = extent;
        m_Frames.Resize(backbuffers.Size());
        for (u32 i = 0; i < backbuffers.Size(); i++)
            m_Frames[i].Image = CreateRef<VulkanImage>(backbuffers[i]);

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
        m_Frames.Resize(Renderer::Configuration().MaxFramesInFlight);
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

        vk::Device device = VulkanRenderer::Device();
        device.waitIdle();

        device.destroyImageView(m_DepthImageView);
        m_DepthImage.Destroy();

        for (u32 i = 0; i < m_Frames.Size(); i++)
        {
            device.destroyFramebuffer(m_Frames[i].Framebuffer, VK_NULL_HANDLE);
            device.destroyImageView(m_Frames[i].ImageView, VK_NULL_HANDLE);
            if (!m_SwapChainTarget) m_Frames[i].Image->Destroy();
        }
        device.destroyRenderPass(m_RenderPass);
    }

    void VulkanFramebuffer::OnResize(u32 width, u32 height)
    {
        vk::Device vkDevice = VulkanRenderer::Device();
        vkDevice.waitIdle();

        Destroy();
        m_Extent = vk::Extent2D(width, height);
        Create();

        vkDevice.waitIdle();
    }
    void VulkanFramebuffer::NextImage()
    {
        u32 framesInFlight  = Renderer::Configuration().MaxFramesInFlight;
        m_CurrentImageIndex = (m_CurrentImageIndex + 1) % framesInFlight;
    }

    void VulkanFramebuffer::CreateImages()
    {
        auto usage = vk::ImageUsageFlagBits::eColorAttachment
                   | vk::ImageUsageFlagBits::eSampled;
        for (u32 i = 0; i < m_Frames.Size(); i++)
        {
            m_Frames[i].Image = CreateRef<VulkanImage>(
                m_Extent.width, m_Extent.height, m_ImageFormat,
                vk::ImageTiling::eOptimal, usage);
            m_Frames[i].Image->TransitionLayout(vk::ImageLayout::eUndefined,
                                               vk::ImageLayout::eShaderReadOnlyOptimal);
        }
    }
    void VulkanFramebuffer::CreateImageViews()
    {
        vk::Device device = VulkanRenderer::Device();

        for (size_t i = 0; i < m_Frames.Size(); i++)
        {
            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.sType        = vk::StructureType::eImageViewCreateInfo;
            viewInfo.pNext        = VK_NULL_HANDLE;
            viewInfo.flags        = vk::ImageViewCreateFlags();
            viewInfo.image        = *m_Frames[i].Image;
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
            = VulkanRenderer::PhysicalDevice().FindDepthFormat();

        vk::Device device = VulkanRenderer::Device();
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
        vk::Device                device = VulkanRenderer::Device();

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
            = VulkanRenderer::PhysicalDevice().FindDepthFormat();
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

        vk::SubpassDependency deps[2]              = {};
        deps[0].srcSubpass                         = VK_SUBPASS_EXTERNAL;
        deps[0].dstSubpass                         = 0;
        deps[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
        deps[0].dstStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        deps[0].srcAccessMask   = vk::AccessFlagBits::eShaderRead;
        deps[0].dstAccessMask   = vk::AccessFlagBits::eColorAttachmentWrite;
        deps[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;
        deps[1].srcSubpass      = 0;
        deps[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        deps[1].srcStageMask
            = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        deps[1].dstStageMask    = vk::PipelineStageFlagBits::eFragmentShader;
        deps[1].srcAccessMask   = vk::AccessFlagBits::eColorAttachmentWrite;
        deps[1].dstAccessMask   = vk::AccessFlagBits::eShaderRead;
        deps[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

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
        (void)dependency;

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
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies   = deps;

        VkCall(device.createRenderPass(&renderPassInfo, VK_NULL_HANDLE,
                                       &m_RenderPass));
    }
    void VulkanFramebuffer::CreateFramebuffers()
    {
        vk::Device device = VulkanRenderer::Device();

        for (u32 i = 0; i < m_Frames.Size(); i++)
        {
            Array<vk::ImageView, 2> attachments
                = {m_Frames[i].ImageView, m_DepthImageView};

            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            framebufferInfo.pNext = VK_NULL_HANDLE;
            framebufferInfo.flags = vk::FramebufferCreateFlags();
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = attachments.Size();
            framebufferInfo.pAttachments    = attachments.Raw();
            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = 1;

            VkCall(device.createFramebuffer(&framebufferInfo, VK_NULL_HANDLE,
                                            &m_Frames[i].Framebuffer));
        }
    }
    void VulkanFramebuffer::CreateSamplers()
    {
        vk::SamplerCreateInfo samplerInfo{};
        (void)samplerInfo;
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

        for (auto& frame : m_Frames) frame.Image->CreateSampler();
    }
}; // namespace Vortex
