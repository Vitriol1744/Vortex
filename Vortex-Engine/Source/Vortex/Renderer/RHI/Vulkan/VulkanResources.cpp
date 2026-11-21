/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanResources.hpp>

namespace Vortex::Vulkan
{
    using namespace RHI;
    using Deletion = DeferredDeletion::Type;

    //------------------------------------------------------------------------
    // Buffer ----------------------------------------------------------------
    //------------------------------------------------------------------------

    VulkanBuffer::VulkanBuffer(const BufferDesc& desc)
        : Buffer(desc)
    {
        VtCoreAssertMsg(desc.Size > 0, "Vulkan: Buffer of size 0");
        auto& device = VulkanDevice::Get();

        vk::BufferCreateInfo info{};
        info.size        = desc.Size;
        info.usage       = ToVk(desc.Usage);
        info.sharingMode = vk::SharingMode::eExclusive;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        switch (desc.Memory)
        {
            case MemoryUsage::eGpuOnly:
                allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
                break;
            case MemoryUsage::eCpuToGpu:
                allocInfo.flags
                    = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                    | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                break;
            case MemoryUsage::eGpuToCpu:
                allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                break;
        }

        VmaAllocationInfo result{};
        VkCall(vmaCreateBuffer(device.Allocator(),
                               reinterpret_cast<VkBufferCreateInfo*>(&info),
                               &allocInfo,
                               reinterpret_cast<VkBuffer*>(&m_Buffer),
                               &m_Allocation, &result));
        VtCoreAssertMsg(m_Buffer, "Vulkan: Failed to allocate a buffer");
        m_Mapped = result.pMappedData;
        SetDebugName(device.Handle(), m_Buffer, desc.DebugName);
        // Named so a leak report can say whose memory outlived the device.
        if (!desc.DebugName.Empty())
            vmaSetAllocationName(device.Allocator(), m_Allocation,
                                 String(desc.DebugName).Raw());
    }
    VulkanBuffer::~VulkanBuffer()
    {
        VulkanDevice::Get().Defer(Deletion::eBuffer, m_Buffer, m_Allocation);
    }

    void* VulkanBuffer::Mapped()
    {
        VtCoreAssertMsg(m_Mapped,
                        "Vulkan: Mapped() on a buffer that isn't host-visible");
        return m_Mapped;
    }

    //------------------------------------------------------------------------
    // Texture ---------------------------------------------------------------
    //------------------------------------------------------------------------

    namespace
    {
        vk::ImageViewType ViewTypeOf(const TextureDesc& desc)
        {
            switch (desc.Type)
            {
                case TextureType::e2D:
                    return desc.Layers > 1 ? vk::ImageViewType::e2DArray
                                           : vk::ImageViewType::e2D;
                case TextureType::e2DArray: return vk::ImageViewType::e2DArray;
                case TextureType::eCube: return vk::ImageViewType::eCube;
                case TextureType::e3D: return vk::ImageViewType::e3D;
            }
            return vk::ImageViewType::e2D;
        }

        void ValidateTextureDesc(const TextureDesc& desc)
        {
            VtCoreAssertMsg(desc.Extent.Width > 0 && desc.Extent.Height > 0,
                            "Vulkan: Texture with an empty extent");
            VtCoreAssertMsg(desc.MipLevels >= 1
                                && desc.MipLevels <= FullMipChain(desc.Size2D()),
                            "Vulkan: Texture mip count out of range");
            VtCoreAssertMsg(desc.Type != TextureType::eCube
                                || desc.Layers % 6 == 0,
                            "Vulkan: A cube texture needs 6 layers per cube");
            VtCoreAssertMsg(desc.Usage != TextureUsage::eNone,
                            "Vulkan: Texture without usage");
        }
    } // namespace

    VulkanTexture::VulkanTexture(const TextureDesc& desc)
        : Texture(desc)
    {
        ValidateTextureDesc(desc);
        auto& device = VulkanDevice::Get();
        m_Aspect     = AspectOf(desc.Format);

        vk::ImageCreateInfo info{};
        info.imageType = desc.Type == TextureType::e3D ? vk::ImageType::e3D
                                                       : vk::ImageType::e2D;
        info.format    = ToVk(desc.Format);
        info.extent    = vk::Extent3D(desc.Extent.Width, desc.Extent.Height,
                                      desc.Extent.Depth);
        info.mipLevels   = desc.MipLevels;
        info.arrayLayers = desc.Layers;
        info.samples     = static_cast<vk::SampleCountFlagBits>(desc.Samples);
        info.tiling      = vk::ImageTiling::eOptimal;
        info.usage       = ToVk(desc.Usage);
        info.sharingMode = vk::SharingMode::eExclusive;
        info.initialLayout = vk::ImageLayout::eUndefined;
        if (desc.Type == TextureType::eCube)
            info.flags = vk::ImageCreateFlagBits::eCubeCompatible;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        // Render targets get their own memory block: they are large and
        // resized often, which would fragment shared blocks.
        if (HasAny(desc.Usage, TextureUsage::eColorAttachment
                                   | TextureUsage::eDepthStencilAttachment))
            allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        VkCall(vmaCreateImage(device.Allocator(),
                              reinterpret_cast<VkImageCreateInfo*>(&info),
                              &allocInfo, reinterpret_cast<VkImage*>(&m_Image),
                              &m_Allocation, nullptr));
        VtCoreAssertMsg(m_Image, "Vulkan: Failed to allocate a texture");
        SetDebugName(device.Handle(), m_Image, desc.DebugName);
        if (!desc.DebugName.Empty())
            vmaSetAllocationName(device.Allocator(), m_Allocation,
                                 String(desc.DebugName).Raw());

        m_View = CreateView(ViewTypeOf(desc), 0, desc.MipLevels, 0,
                            desc.Layers, true);
    }

    VulkanTexture::VulkanTexture(const TextureDesc& desc, vk::Image image)
        : Texture(desc)
        , m_Image(image)
        , m_Aspect(AspectOf(desc.Format))
        , m_Owned(false)
    {
        m_View = CreateView(vk::ImageViewType::e2D, 0, 1, 0, 1, false);
        SetDebugName(VulkanDevice::Get().Handle(), m_Image, desc.DebugName);
    }

    VulkanTexture::~VulkanTexture()
    {
        auto& device = VulkanDevice::Get();
        for (auto& view : m_SubresourceViews)
            device.Defer(Deletion::eImageView, view.View);
        device.Defer(Deletion::eImageView, m_View);
        if (m_Owned) device.Defer(Deletion::eImage, m_Image, m_Allocation);
    }

    vk::ImageView VulkanTexture::View(u32 mip, u32 layer)
    {
        VtCoreAssertMsg(mip < m_Desc.MipLevels && layer < m_Desc.Layers,
                        "Vulkan: Texture subresource out of range");
        // The default view doubles as the attachment view unless sampling
        // would restrict its aspect (combined depth/stencil).
        bool restricted = HasStencil(m_Desc.Format)
                       && HasAny(m_Desc.Usage, TextureUsage::eSampled);
        if (m_Desc.MipLevels == 1 && m_Desc.Layers == 1 && !restricted)
            return m_View;

        std::lock_guard lock(m_ViewMutex);
        for (const auto& view : m_SubresourceViews)
            if (!view.Storage && view.Mip == mip && view.Layer == layer)
                return view.View;

        vk::ImageView view
            = CreateView(vk::ImageViewType::e2D, mip, 1, layer, 1, false);
        m_SubresourceViews.PushBack({mip, layer, view, false});
        return view;
    }

    vk::ImageView VulkanTexture::StorageView(u32 mip)
    {
        VtCoreAssertMsg(mip < m_Desc.MipLevels, "Vulkan: Storage mip out of range");
        VtCoreAssertMsg(HasAny(m_Desc.Usage, TextureUsage::eStorage),
                        "Vulkan: Storage view of a texture without Storage usage");
        std::lock_guard lock(m_ViewMutex);
        for (const auto& view : m_SubresourceViews)
            if (view.Storage && view.Mip == mip) return view.View;

        auto type = m_Desc.Layers > 1 ? vk::ImageViewType::e2DArray
                                      : vk::ImageViewType::e2D;
        vk::ImageView view = CreateView(type, mip, 1, 0, m_Desc.Layers, false);
        m_SubresourceViews.PushBack({mip, 0, view, true});
        return view;
    }

    vk::ImageView VulkanTexture::CreateView(vk::ImageViewType type, u32 baseMip,
                                            u32 mipCount, u32 baseLayer,
                                            u32 layerCount, bool sampling)
    {
        vk::ImageViewCreateInfo info{};
        info.image    = m_Image;
        info.viewType = type;
        info.format   = ToVk(m_Desc.Format);
        info.subresourceRange.aspectMask     = m_Aspect;
        info.subresourceRange.baseMipLevel   = baseMip;
        info.subresourceRange.levelCount     = mipCount;
        info.subresourceRange.baseArrayLayer = baseLayer;
        info.subresourceRange.layerCount     = layerCount;

        // Sampling a combined depth/stencil image reads depth only.
        if (sampling && HasStencil(m_Desc.Format))
            info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        vk::ImageView view   = VK_NULL_HANDLE;
        auto&         device = VulkanDevice::Get();
        VkCall(device.Handle().createImageView(&info, nullptr, &view));
        return view;
    }

    //------------------------------------------------------------------------
    // Sampler ---------------------------------------------------------------
    //------------------------------------------------------------------------

    VulkanSampler::VulkanSampler(const SamplerDesc& desc)
        : Sampler(desc)
    {
        auto& device = VulkanDevice::Get();

        vk::SamplerCreateInfo info{};
        info.minFilter    = ToVk(desc.Min);
        info.magFilter    = ToVk(desc.Mag);
        info.mipmapMode   = ToVkMipmapMode(desc.Mip);
        info.addressModeU = ToVk(desc.AddressU);
        info.addressModeV = ToVk(desc.AddressV);
        info.addressModeW = ToVk(desc.AddressW);
        info.minLod       = desc.MinLod;
        info.maxLod       = desc.MaxLod;
        info.borderColor  = vk::BorderColor::eFloatOpaqueBlack;

        f32 maxAnisotropy = desc.MaxAnisotropy;
        if (maxAnisotropy > device.Limits().MaxSamplerAnisotropy)
            maxAnisotropy = device.Limits().MaxSamplerAnisotropy;
        if (device.Features().SamplerAnisotropy && maxAnisotropy > 1.0f)
        {
            info.anisotropyEnable = VK_TRUE;
            info.maxAnisotropy    = maxAnisotropy;
        }

        VkCall(device.Handle().createSampler(&info, nullptr, &m_Sampler));
        SetDebugName(device.Handle(), m_Sampler, desc.DebugName);
    }
    VulkanSampler::~VulkanSampler()
    {
        VulkanDevice::Get().Defer(Deletion::eSampler, m_Sampler);
    }
} // namespace Vortex::Vulkan
