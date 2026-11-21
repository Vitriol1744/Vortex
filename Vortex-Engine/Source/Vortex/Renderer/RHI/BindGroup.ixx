/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

export module Vortex.RHI.BindGroup;

export import Vortex.RHI.Resources;

import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;
import Prism.String.StringView;

/**
 * Resource binding. A BindGroupLayout describes the bindings of one set
 * (`[[vk::binding(M, N)]]` in HLSL); shaders produce them by
 * reflection. A BindGroup is an immutable set of resources matching a layout,
 * bound with CommandList::BindGroup. To change what is bound, create a new
 * group: they are pooled and cheap, and the old one is freed once the GPU is
 * done with it (updating a group the GPU may be reading would be a race).
 */
export namespace Vortex::RHI
{
    enum class BindingType : u8
    {
        eUniformBuffer,
        eStorageBuffer,
        /// Texture and sampler in one binding (any HLSL Texture*, GLSL
        /// `sampler2D`).
        eSampledTexture,
        /// GLSL `texture2D`, paired with a separate eSampler binding (HLSL
        /// never produces it: its samplers are compiled out).
        eTexture,
        eSampler,
        eStorageTexture,
    };

    constexpr bool IsBufferBinding(BindingType type)
    {
        return type == BindingType::eUniformBuffer
            || type == BindingType::eStorageBuffer;
    }

    struct BindingLayoutEntry
    {
        u32         Binding = 0;
        BindingType Type    = BindingType::eUniformBuffer;
        /// Array size (1 for a single resource).
        u32         Count   = 1;
        ShaderStage Stages  = ShaderStage::eNone;
        /// Size of the block for buffers, as declared in the shader (0 if the
        /// block has a runtime-sized array).
        u32         Size    = 0;
        String      Name{};
    };

    struct BindGroupLayoutDesc
    {
        Vector<BindingLayoutEntry> Entries;

        inline const BindingLayoutEntry* Find(u32 binding) const
        {
            for (const auto& entry : Entries)
                if (entry.Binding == binding) return &entry;
            return nullptr;
        }
        inline const BindingLayoutEntry* Find(StringView name) const
        {
            for (const auto& entry : Entries)
                if (entry.Name == name) return &entry;
            return nullptr;
        }
    };

    class VT_RENDERER_RHI_API BindGroupLayout
        : public RefCounted<BindGroupLayout>
    {
      public:
        BindGroupLayout(const BindGroupLayout&)            = delete;
        BindGroupLayout& operator=(const BindGroupLayout&) = delete;
        virtual ~BindGroupLayout()                         = default;

        inline const BindGroupLayoutDesc& Desc() const { return m_Desc; }

      protected:
        explicit BindGroupLayout(const BindGroupLayoutDesc& desc)
            : m_Desc(desc)
        {
        }

        BindGroupLayoutDesc m_Desc;
    };

    /// One resource bound to one binding (or one element of an array).
    struct BindGroupEntry
    {
        u32           Binding      = 0;
        u32           ArrayElement = 0;

        // Buffer bindings
        ::Ref<RHI::Buffer> Buffer  = nullptr;
        usize         Offset       = 0;
        /// 0 = from Offset to the end of the buffer.
        usize         Range        = 0;

        // Texture / sampler bindings
        ::Ref<RHI::Texture> Texture = nullptr;
        ::Ref<RHI::Sampler> Sampler = nullptr;
        /// Storage textures: the mip level bound (as one image covering
        /// every layer, so a cube map is an image2DArray in the shader).
        u32                 MipLevel = 0;

        static inline BindGroupEntry Uniform(u32 binding, ::Ref<RHI::Buffer> buffer,
                                             usize offset = 0, usize range = 0)
        {
            BindGroupEntry entry;
            entry.Binding = binding;
            entry.Buffer  = buffer;
            entry.Offset  = offset;
            entry.Range   = range;
            return entry;
        }
        static inline BindGroupEntry
        StorageTexture(u32 binding, ::Ref<RHI::Texture> texture, u32 mipLevel = 0)
        {
            BindGroupEntry entry;
            entry.Binding  = binding;
            entry.Texture  = texture;
            entry.MipLevel = mipLevel;
            return entry;
        }
        static inline BindGroupEntry
        SampledTexture(u32 binding, ::Ref<RHI::Texture> texture,
                       ::Ref<RHI::Sampler> sampler)
        {
            BindGroupEntry entry;
            entry.Binding = binding;
            entry.Texture = texture;
            entry.Sampler = sampler;
            return entry;
        }
    };

    struct BindGroupDesc
    {
        ::Ref<BindGroupLayout> Layout = nullptr;
        Vector<BindGroupEntry> Entries;
        String                 DebugName{};
    };

    class VT_RENDERER_RHI_API BindGroup : public RefCounted<BindGroup>
    {
      public:
        BindGroup(const BindGroup&)            = delete;
        BindGroup& operator=(const BindGroup&) = delete;
        virtual ~BindGroup()                   = default;

        inline const BindGroupDesc&   Desc() const { return m_Desc; }
        inline ::Ref<BindGroupLayout> Layout() const { return m_Desc.Layout; }

      protected:
        /// Keeps the bound resources alive as long as the group exists.
        explicit BindGroup(const BindGroupDesc& desc)
            : m_Desc(desc)
        {
        }

        BindGroupDesc m_Desc;
    };
} // namespace Vortex::RHI
