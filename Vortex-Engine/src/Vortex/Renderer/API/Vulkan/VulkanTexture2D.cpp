/*
 * Created by v1tr10l7 on 20.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanTexture2D.hpp"

namespace Vortex
{
    VulkanTexture2D::VulkanTexture2D(PathView path)
    {
        Image          image(path);
        usize          size       = image.GetSize();
        constexpr bool useMipMaps = true;
        u32            mipLevels  = static_cast<u32>(std::floor(std::log2(
                            std::max(image.GetWidth(), image.GetHeight()))))
                      + 1;
        if (!useMipMaps) mipLevels = 1;
        m_Size = size;

        vk::BufferCreateInfo stagingInfo{};
        stagingInfo.sType       = vk::StructureType::eBufferCreateInfo;
        stagingInfo.size        = size;
        stagingInfo.usage       = vk::BufferUsageFlagBits::eTransferSrc;
        stagingInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::Buffer    stagingBuffer{};
        VmaAllocation stagingAllocation = VulkanAllocator::AllocateBuffer(
            stagingInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

        u8* dest = VulkanAllocator::MapMemory<u8*>(stagingAllocation);
        std::memcpy(dest, image.GetPixels(), size);
        VulkanAllocator::UnmapMemory(stagingAllocation);

        m_TextureImage.Create(image.GetWidth(), image.GetHeight(),
                              vk::Format::eR8G8B8A8Srgb,
                              vk::ImageTiling::eOptimal,
                              vk::ImageUsageFlagBits::eTransferSrc
                                  | vk::ImageUsageFlagBits::eTransferDst
                                  | vk::ImageUsageFlagBits::eSampled,
                              mipLevels);

        m_TextureImage.TransitionLayout(vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        mipLevels);
        m_TextureImage.CopyFrom(stagingBuffer, image.GetWidth(),
                                image.GetHeight());
        // TODO(v1tr10l7): Check support for mip mapping
        m_TextureImage.TransitionLayout(vk::ImageLayout::eUndefined,
                                        vk::ImageLayout::eTransferDstOptimal,
                                        mipLevels);
        m_TextureImage.GenerateMipMaps(image.GetWidth(), image.GetHeight(),
                                       mipLevels);

        VulkanAllocator::DestroyBuffer(stagingBuffer, stagingAllocation);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.sType    = vk::StructureType::eImageViewCreateInfo;
        viewInfo.image    = m_TextureImage;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format   = vk::Format::eR8G8B8A8Srgb;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.levelCount     = mipLevels;
        viewInfo.subresourceRange.layerCount     = 1;

        vk::Device device                        = VulkanRenderer::GetDevice();
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
        samplerInfo.maxLod = useMipMaps ? static_cast<f32>(mipLevels) : 0.0f;
        VkCall(device.createSampler(&samplerInfo, VK_NULL_HANDLE,
                                    &m_TextureSampler));

        m_ImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        m_ImageInfo.imageView   = m_TextureView;
        m_ImageInfo.sampler     = m_TextureSampler;
    }
    VulkanTexture2D::~VulkanTexture2D()
    {
        vk::Device device = VulkanRenderer::GetDevice();

        device.destroySampler(m_TextureSampler);
        device.destroyImageView(m_TextureView);
        m_TextureImage.Destroy();
    }
}; // namespace Vortex
