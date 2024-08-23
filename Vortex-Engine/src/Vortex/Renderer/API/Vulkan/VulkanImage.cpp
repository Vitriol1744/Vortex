/*
 * Created by v1tr10l7 on 23.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanImage.hpp"

namespace Vortex
{
    void VulkanImage::Create(u32 width, u32 height, vk::Format format,
                             vk::ImageTiling tiling, vk::ImageUsageFlags usage)
    {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.sType                 = vk::StructureType::eImageCreateInfo;
        imageInfo.pNext                 = VK_NULL_HANDLE;
        imageInfo.flags                 = vk::ImageCreateFlags();
        imageInfo.imageType             = vk::ImageType::e2D;
        imageInfo.format                = format;
        imageInfo.extent.width          = width;
        imageInfo.extent.height         = height;
        imageInfo.extent.depth          = 1;
        imageInfo.mipLevels             = 1;
        imageInfo.arrayLayers           = 1;
        imageInfo.samples               = vk::SampleCountFlagBits::e1;
        imageInfo.tiling                = tiling;
        imageInfo.usage                 = usage;
        imageInfo.sharingMode           = vk::SharingMode::eExclusive;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;
        imageInfo.initialLayout         = vk::ImageLayout::eUndefined;

        m_Allocation                    = VulkanAllocator::AllocateImage(
            imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_Image, m_Size);
    }
    void VulkanImage::Destroy()
    {
        if (m_Allocation)
        {
            VulkanAllocator::DestroyImage(m_Image, m_Allocation);
            m_Image      = VK_NULL_HANDLE;
            m_Allocation = VK_NULL_HANDLE;
        }
    }

    void VulkanImage::CopyFrom(vk::Buffer buffer, u32 width, u32 height)
    {
        TransitionLayout(vk::ImageLayout::eUndefined,
                         vk::ImageLayout::eTransferDstOptimal);

        const VulkanDevice& device        = VulkanContext::GetDevice();
        vk::CommandBuffer   commandBuffer = device.BeginTransferCommand();

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
            buffer, m_Image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
        device.EndTransferCommand(commandBuffer);

        TransitionLayout(vk::ImageLayout::eTransferDstOptimal,
                         vk::ImageLayout::eShaderReadOnlyOptimal);
    }
    void VulkanImage::TransitionLayout(vk::ImageLayout oldLayout,
                                       vk::ImageLayout newLayout)
    {
        const VulkanDevice& device        = VulkanContext::GetDevice();
        vk::CommandBuffer   commandBuffer = device.BeginOneTimeRenderCommand();

        vk::ImageMemoryBarrier barrier{};
        barrier.sType               = vk::StructureType::eImageMemoryBarrier;
        barrier.pNext               = VK_NULL_HANDLE;
        barrier.srcAccessMask       = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask       = vk::AccessFlagBits::eNone;
        barrier.oldLayout           = oldLayout;
        barrier.newLayout           = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = m_Image;
        barrier.subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount   = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;

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
        else if (oldLayout == vk::ImageLayout::eUndefined
                 && newLayout
                        == vk::ImageLayout::eDepthStencilAttachmentOptimal)
        {
            barrier.dstAccessMask
                = vk::AccessFlagBits::eDepthStencilAttachmentRead
                | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

            barrier.subresourceRange.aspectMask
                = vk::ImageAspectFlagBits::eDepth;

            barrier.subresourceRange.aspectMask
                |= vk::ImageAspectFlagBits::eStencil;
        }
        else VtCoreAssert(false);

        commandBuffer.pipelineBarrier(srcStage, dstStage,
                                      vk::DependencyFlags(0), 0, VK_NULL_HANDLE,
                                      0, VK_NULL_HANDLE, 1, &barrier);

        device.EndOneTimeRenderCommand(commandBuffer);
    }
}; // namespace Vortex
