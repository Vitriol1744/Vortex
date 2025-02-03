/*
 * Created by v1tr10l7 on 23.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanImage.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"

namespace Vortex
{
    void VulkanImage::Create(u32 width, u32 height, vk::Format format,
                             vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                             u32 mipLevels)
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
        imageInfo.mipLevels             = mipLevels;
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
        m_MipLevels = mipLevels;
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
        const VulkanDevice& device        = VulkanRenderer::GetDevice();
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
    }
    void VulkanImage::TransitionLayout(vk::ImageLayout oldLayout,
                                       vk::ImageLayout newLayout, u32 mipLevels)
    {
        const VulkanDevice& device        = VulkanRenderer::GetDevice();
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
        barrier.subresourceRange.levelCount   = mipLevels;
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

    void VulkanImage::GenerateMipMaps(i32 width, i32 height, u32 mipLevels)
    {
        auto&             device        = VulkanRenderer::GetDevice();
        vk::CommandBuffer commandBuffer = device.BeginOneTimeRenderCommand();

        vk::ImageMemoryBarrier barrier{};
        barrier.sType               = vk::StructureType::eImageMemoryBarrier;
        barrier.image               = m_Image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseMipLevel   = 0;

        i32 mipWidth                            = width;
        i32 mipHeight                           = height;
        for (u32 i = 1; i < mipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout     = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0,
                VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

            vk::ImageBlit blit{};
            blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
            blit.srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
            blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.srcSubresource.mipLevel   = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = 1;
            blit.dstOffsets[0]                 = vk::Offset3D(0, 0, 0);
            blit.dstOffsets[1]
                = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1,
                               mipHeight > 1 ? mipHeight / 2 : 1, 1);
            blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            blit.dstSubresource.mipLevel   = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = 1;
            commandBuffer.blitImage(
                m_Image, vk::ImageLayout::eTransferSrcOptimal, m_Image,
                vk::ImageLayout::eTransferDstOptimal, 1, &blit,
                vk::Filter::eLinear);
            barrier.oldLayout     = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlags(), 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1,
                &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout     = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout     = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(),
            0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);
        device.EndOneTimeRenderCommand(commandBuffer);
    }
}; // namespace Vortex
