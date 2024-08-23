/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp"

namespace Vortex
{
    VulkanTexture2D::VulkanTexture2D(PathView path)
        : Texture2D(path)
    {
        usize                size = m_Image.GetSize();

        vk::BufferCreateInfo stagingInfo{};
        stagingInfo.sType       = vk::StructureType::eBufferCreateInfo;
        stagingInfo.size        = size;
        stagingInfo.usage       = vk::BufferUsageFlagBits::eTransferSrc;
        stagingInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::Buffer    stagingBuffer{};
        VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(
            stagingInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

        u8* dest = VulkanAllocator::MapMemory<u8*>(stagingAllocation);
        std::memcpy(dest, m_Image.GetPixels(), size);
        VulkanAllocator::UnmapMemory(stagingAllocation);

        vk::ImageCreateInfo imageInfo{};
        imageInfo.sType         = vk::StructureType::eImageCreateInfo;
        imageInfo.imageType     = vk::ImageType::e2D;
        imageInfo.extent.width  = m_Image.GetWidth();
        imageInfo.extent.height = m_Image.GetHeight();
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = vk::Format::eR8G8B8A8Srgb;
        imageInfo.tiling        = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage         = vk::ImageUsageFlagBits::eTransferDst
                        | vk::ImageUsageFlagBits::eSampled;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples     = vk::SampleCountFlagBits::e1;
        imageInfo.flags       = vk::ImageCreateFlagBits();

        m_Allocation          = VulkanAllocator::AllocateImage(
            imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_TextureImage, m_Size);

        // VkCall(VulkanAllocator::BindImageMemory(m_Allocation,
        // m_TextureImage));
        TransitionImageLayout(m_TextureImage, vk::Format::eR8G8B8A8Srgb,
                              vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal);
        CopyBufferToImage(stagingBuffer, m_TextureImage, m_Image.GetWidth(),
                          m_Image.GetHeight());
        TransitionImageLayout(m_TextureImage, vk::Format::eR8G8B8A8Srgb,
                              vk::ImageLayout::eTransferDstOptimal,
                              vk::ImageLayout::eShaderReadOnlyOptimal);
        VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.sType    = vk::StructureType::eImageViewCreateInfo;
        viewInfo.image    = m_TextureImage;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format   = vk::Format::eR8G8B8A8Srgb;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.layerCount     = 1;

        vk::Device device                        = VulkanContext::GetDevice();
        VkCall(
            device.createImageView(&viewInfo, VK_NULL_HANDLE, &m_TextureView));

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
        VkCall(device.createSampler(&samplerInfo, VK_NULL_HANDLE,
                                    &m_TextureSampler));

        m_ImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        m_ImageInfo.imageView   = m_TextureView;
        m_ImageInfo.sampler     = m_TextureSampler;

        // TODO(v1tr10l7): this function sits here temporarily
        stbi_image_free(m_Image.GetPixels());
    }
    VulkanTexture2D::~VulkanTexture2D()
    {
        vk::Device device = VulkanContext::GetDevice();

        device.destroySampler(m_TextureSampler);
        device.destroyImageView(m_TextureView);
        VulkanAllocator::DestroyImage(m_TextureImage, m_Allocation);
    }

    void VulkanTexture2D::TransitionImageLayout(vk::Image       image,
                                                vk::Format      format,
                                                vk::ImageLayout oldLayout,
                                                vk::ImageLayout newLayout)
    {
        (void)format;
        vk::Device                device = VulkanContext::GetDevice();
        // TODO(v1tr10l7): TEMPORARY:
        vk::CommandPool           commandPool{};
        vk::CommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        poolCreateInfo.pNext = VK_NULL_HANDLE;
        poolCreateInfo.flags
            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolCreateInfo.queueFamilyIndex = VulkanContext::GetPhysicalDevice()
                                              .GetQueueFamilyIndices()
                                              .Graphics.value();
        VkCall(device.createCommandPool(&poolCreateInfo, VK_NULL_HANDLE,
                                        &commandPool));

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.level       = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer{};
        VkCall(device.allocateCommandBuffers(&allocInfo, &commandBuffer));

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        VkCall(commandBuffer.begin(&beginInfo));

        vk::ImageMemoryBarrier barrier = {};
        barrier.sType                  = vk::StructureType::eImageMemoryBarrier;
        barrier.oldLayout              = oldLayout;
        barrier.newLayout              = newLayout;
        barrier.srcQueueFamilyIndex    = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex    = vk::QueueFamilyIgnored;
        barrier.image                  = image;
        barrier.subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount   = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.srcAccessMask                   = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask                   = vk::AccessFlagBits::eNone;

        vk::PipelineStageFlagBits srcStage;
        vk::PipelineStageFlagBits dstStage;

        if (oldLayout == vk::ImageLayout::eUndefined
            && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            srcStage              = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage              = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
                 && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            srcStage              = vk::PipelineStageFlagBits::eTransfer;
            dstStage              = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else VtCoreAssert(false);

        commandBuffer.pipelineBarrier(
            srcStage, dstStage, vk::DependencyFlagBits(0), 0, VK_NULL_HANDLE, 0,
            VK_NULL_HANDLE, 1, &barrier);

        commandBuffer.end();

        vk::SubmitInfo submit{};
        submit.sType              = vk::StructureType::eSubmitInfo;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers    = &commandBuffer;

        vk::Queue queue = VulkanContext::GetDevice().GetGraphicsQueue();
        VkCall(queue.submit(1, &submit, VK_NULL_HANDLE));
        queue.waitIdle();

        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
        device.destroyCommandPool(commandPool);
    }
    void VulkanTexture2D::CopyBufferToImage(vk::Buffer buffer, vk::Image image,
                                            u32 width, u32 height)
    {
        vk::Device                device = VulkanContext::GetDevice();
        // TODO(v1tr10l7): TEMPORARY:
        vk::CommandPool           commandPool{};
        vk::CommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        poolCreateInfo.pNext = VK_NULL_HANDLE;
        poolCreateInfo.flags
            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolCreateInfo.queueFamilyIndex = VulkanContext::GetPhysicalDevice()
                                              .GetQueueFamilyIndices()
                                              .Transfer.value();
        VkCall(device.createCommandPool(&poolCreateInfo, VK_NULL_HANDLE,
                                        &commandPool));

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.level       = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer{};
        VkCall(device.allocateCommandBuffers(&allocInfo, &commandBuffer));

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        VkCall(commandBuffer.begin(&beginInfo));

        vk::BufferImageCopy region{};
        region.bufferOffset                = 0;
        region.bufferRowLength             = 0;
        region.bufferImageHeight           = 0;

        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel   = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;

        region.imageOffset                     = vk::Offset3D(0, 0, 0);
        region.imageExtent                     = vk::Extent3D(width, height, 1);

        commandBuffer.copyBufferToImage(
            buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        commandBuffer.end();

        vk::SubmitInfo submit{};
        submit.sType              = vk::StructureType::eSubmitInfo;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers    = &commandBuffer;

        vk::Queue transferQueue = VulkanContext::GetDevice().GetTransferQueue();
        VkCall(transferQueue.submit(1, &submit, VK_NULL_HANDLE));
        transferQueue.waitIdle();

        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
        device.destroyCommandPool(commandPool);
    }
}; // namespace Vortex
