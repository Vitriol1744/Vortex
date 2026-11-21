/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

export module Vortex.RHI.Resources;

export import Vortex.RHI.Types;

import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;
import Prism.String.StringView;

/**
 * GPU resources. They are created by RHI::Device and reference counted;
 * dropping the last Ref is always safe, even while the GPU still uses the
 * resource: backends hand the native objects to the device's deferred
 * deletion queue, which frees them once every frame that could reference them
 * has completed.
 *
 * Resources are immutable in shape (size, format, usage) after creation.
 */
export namespace Vortex::RHI
{
    //------------------------------------------------------------------------
    // Buffer ----------------------------------------------------------------
    //------------------------------------------------------------------------

    struct BufferDesc
    {
        usize       Size   = 0;
        BufferUsage Usage  = BufferUsage::eNone;
        MemoryUsage Memory = MemoryUsage::eGpuOnly;
        String      DebugName{};
    };

    class VT_RENDERER_RHI_API Buffer : public RefCounted<Buffer>
    {
      public:
        Buffer(const Buffer&)            = delete;
        Buffer& operator=(const Buffer&) = delete;
        virtual ~Buffer()                = default;

        inline const BufferDesc& Desc() const { return m_Desc; }
        inline usize             Size() const { return m_Desc.Size; }
        inline bool              IsHostVisible() const
        {
            return m_Desc.Memory != MemoryUsage::eGpuOnly;
        }

        /// Persistently mapped memory of a host-visible buffer. It stays
        /// valid for the buffer's lifetime; there is no Unmap.
        virtual void* Mapped() = 0;

        /// Copies into a host-visible buffer. The CPU writes immediately, so
        /// don't overwrite data the GPU may still read (use one buffer per
        /// frame in flight, or Device::UploadBuffer for GPU-only buffers).
        inline void   Write(const void* data, usize size, usize offset = 0)
        {
            VtCoreAssertMsg(IsHostVisible(),
                            "RHI: Buffer::Write on a GPU-only buffer; use "
                            "Device::UploadBuffer");
            VtCoreAssertMsg(offset + size <= m_Desc.Size,
                            "RHI: Buffer::Write out of bounds");
            __builtin_memcpy(static_cast<u8*>(Mapped()) + offset, data, size);
        }

      protected:
        explicit Buffer(const BufferDesc& desc)
            : m_Desc(desc)
        {
        }

        BufferDesc m_Desc;
    };

    //------------------------------------------------------------------------
    // Texture ---------------------------------------------------------------
    //------------------------------------------------------------------------

    /// Mip count that reduces `extent` down to 1x1.
    constexpr u32 FullMipChain(Extent2D extent)
    {
        u32 size   = extent.Width > extent.Height ? extent.Width : extent.Height;
        u32 levels = 1;
        while (size > 1)
        {
            size >>= 1;
            ++levels;
        }
        return levels;
    }

    struct TextureDesc
    {
        TextureType  Type      = TextureType::e2D;
        RHI::Format  Format    = RHI::Format::eRGBA8Unorm;
        Extent3D     Extent    = {};
        u32          MipLevels = 1;
        /// Array layers; 6 for a cube map.
        u32          Layers    = 1;
        u32          Samples   = 1;
        TextureUsage Usage     = TextureUsage::eSampled;
        String       DebugName{};

        constexpr RHI::Extent2D Size2D() const
        {
            return {Extent.Width, Extent.Height};
        }
    };

    class VT_RENDERER_RHI_API Texture : public RefCounted<Texture>
    {
      public:
        Texture(const Texture&)            = delete;
        Texture& operator=(const Texture&) = delete;
        virtual ~Texture()                 = default;

        inline const TextureDesc& Desc() const { return m_Desc; }
        inline RHI::Format        Format() const { return m_Desc.Format; }
        inline Extent3D           Extent() const { return m_Desc.Extent; }
        inline u32                Width() const { return m_Desc.Extent.Width; }
        inline u32  Height() const { return m_Desc.Extent.Height; }
        inline u32  MipLevels() const { return m_Desc.MipLevels; }
        inline u32  Layers() const { return m_Desc.Layers; }

        /**
         * The state the texture is in at the end of what has been recorded
         * so far. Command lists update it as they record barriers, so it is
         * only meaningful on the thread that records commands (the render
         * thread). Whole-resource granularity: per-mip transitions must
         * leave every subresource in one state before the next tracked use.
         */
        inline ResourceState TrackedState() const { return m_State; }
        inline void SetTrackedState(ResourceState state) { m_State = state; }

      protected:
        explicit Texture(const TextureDesc& desc)
            : m_Desc(desc)
        {
        }

        TextureDesc   m_Desc;
        ResourceState m_State = ResourceState::eUndefined;
    };

    //------------------------------------------------------------------------
    // Sampler ---------------------------------------------------------------
    //------------------------------------------------------------------------

    struct SamplerDesc
    {
        Filter      Min           = Filter::eLinear;
        Filter      Mag           = Filter::eLinear;
        Filter      Mip           = Filter::eLinear;
        AddressMode AddressU      = AddressMode::eRepeat;
        AddressMode AddressV      = AddressMode::eRepeat;
        AddressMode AddressW      = AddressMode::eRepeat;
        /// 0 or 1 disables anisotropic filtering. Clamped to the device limit.
        f32         MaxAnisotropy = 16.0f;
        f32         MinLod        = 0.0f;
        f32         MaxLod        = 1000.0f;
        String      DebugName{};
    };

    class VT_RENDERER_RHI_API Sampler : public RefCounted<Sampler>
    {
      public:
        Sampler(const Sampler&)            = delete;
        Sampler& operator=(const Sampler&) = delete;
        virtual ~Sampler()                 = default;

        inline const SamplerDesc& Desc() const { return m_Desc; }

      protected:
        explicit Sampler(const SamplerDesc& desc)
            : m_Desc(desc)
        {
        }

        SamplerDesc m_Desc;
    };
} // namespace Vortex::RHI
