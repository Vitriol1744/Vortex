/*
 * Created by v1tr10l7 on 13.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// Backend-private header of the Vulkan RHI. Include it FIRST in every backend
// source file: it pulls in the C headers (Vulkan, VMA), which have to come
// before any `import` in a translation unit, or clang's global module fragment
// merging reports bogus typedef redefinitions.
//
// Vulkan-Hpp's own C++ module (vulkan.cppm) can't be used: it does
// `export import std;`, and Vortex/Prism don't use the std module.

#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Log/Log.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Vulkan/Export.hpp>

import Prism.String.String;
import Prism.String.StringView;
import Vortex.RHI;

#ifndef VT_DIST
    #define VkCall(x)                                                          \
        do                                                                     \
        {                                                                      \
            vk::Result vkCallStatus_ = static_cast<vk::Result>(x);             \
            if (vkCallStatus_ != vk::Result::eSuccess)                         \
                VtCoreError("Vulkan: '{}' failed: {}", #x,                     \
                            ::Vortex::Vulkan::VkResultToString(               \
                                vkCallStatus_));                               \
        } while (false)
#else
    #define VkCall(x)                                                          \
        do                                                                     \
        {                                                                      \
            (void)(x);                                                         \
        } while (false)
#endif

namespace Vortex::Vulkan
{
    String VkResultToString(vk::Result result);

    //------------------------------------------------------------------------
    // RHI -> Vulkan translation ---------------------------------------------
    //------------------------------------------------------------------------

    vk::Format              ToVk(RHI::Format format);
    RHI::Format             FromVk(vk::Format format);
    vk::BufferUsageFlags    ToVk(RHI::BufferUsage usage);
    vk::ImageUsageFlags     ToVk(RHI::TextureUsage usage);
    vk::ShaderStageFlags    ToVk(RHI::ShaderStage stages);
    vk::ShaderStageFlagBits ToVkStage(RHI::ShaderStage stage);
    RHI::ShaderStage        FromVk(vk::ShaderStageFlags stages);
    vk::DescriptorType      ToVk(RHI::BindingType type);
    vk::PrimitiveTopology   ToVk(RHI::Topology topology);
    vk::PolygonMode         ToVk(RHI::PolygonMode mode);
    vk::CullModeFlags       ToVk(RHI::CullMode mode);
    vk::FrontFace           ToVk(RHI::FrontFace face);
    vk::CompareOp           ToVk(RHI::CompareOp op);
    vk::BlendFactor         ToVk(RHI::BlendFactor factor);
    vk::BlendOp             ToVk(RHI::BlendOp op);
    vk::ColorComponentFlags ToVk(RHI::ColorMask mask);
    vk::Filter              ToVk(RHI::Filter filter);
    vk::SamplerMipmapMode   ToVkMipmapMode(RHI::Filter filter);
    vk::SamplerAddressMode  ToVk(RHI::AddressMode mode);
    vk::AttachmentLoadOp    ToVk(RHI::LoadOp op);
    vk::AttachmentStoreOp   ToVk(RHI::StoreOp op);
    vk::IndexType           ToVk(RHI::IndexType type);
    vk::ImageAspectFlags    AspectOf(RHI::Format format);

    /// Pipeline stages, accesses and image layout a resource state implies.
    struct StateInfo
    {
        vk::PipelineStageFlags2 Stages = vk::PipelineStageFlagBits2::eNone;
        vk::AccessFlags2        Access = vk::AccessFlagBits2::eNone;
        vk::ImageLayout         Layout = vk::ImageLayout::eUndefined;
    };
    /// `asSource` selects the stages a barrier waits on (srcStageMask) rather
    /// than the stages it unblocks.
    StateInfo GetStateInfo(RHI::ResourceState state, bool asSource);

    /// Gives a Vulkan object a name shown by validation messages and GPU
    /// debuggers. No-op without VK_EXT_debug_utils.
    void      SetDebugName(vk::Device device, vk::ObjectType type,
                           u64 handle, StringView name);
    template <typename Handle>
    inline void SetDebugName(vk::Device device, Handle handle, StringView name)
    {
        using CType = typename Handle::CType;
        SetDebugName(device, Handle::objectType,
                     reinterpret_cast<u64>(static_cast<CType>(handle)), name);
    }
} // namespace Vortex::Vulkan
