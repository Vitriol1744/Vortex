/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

export module Vortex.RHI.CommandList;

export import Vortex.RHI.Pipeline;

import Prism.Containers.Span;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.String.StringView;

/**
 * Command recording. A CommandList is obtained from Device::BeginCommandList
 * for the current frame, recorded on one thread (the render thread) and handed
 * back with Device::Submit. Nothing is implicit: render targets, clears,
 * viewport, scissor, bindings and barriers all come from the caller (or from
 * the RenderGraph, which records these for you).
 */
export namespace Vortex::RHI
{
    struct ColorAttachment
    {
        ::Ref<RHI::Texture> Texture  = nullptr;
        LoadOp              Load     = LoadOp::eClear;
        StoreOp             Store    = StoreOp::eStore;
        ClearColor          Clear    = {};
        u32                 MipLevel = 0;
        u32                 Layer    = 0;
    };

    struct DepthAttachment
    {
        ::Ref<RHI::Texture> Texture = nullptr;
        LoadOp              Load    = LoadOp::eClear;
        StoreOp             Store   = StoreOp::eDontCare;
        ClearDepthStencil   Clear   = {};
        /// Bind read-only (depth test without writes, e.g. a later pass that
        /// also samples the depth buffer).
        bool                ReadOnly = false;
    };

    struct RenderingDesc
    {
        Vector<ColorAttachment> Colors;
        DepthAttachment         Depth;
        /// Empty means the full extent of the first attachment.
        Rect2D                  Area = {};
        StringView              DebugName;
    };

    /// All mips / layers when used as a count.
    inline constexpr u32 kAllSubresources = ~0u;

    struct TextureBarrier
    {
        RHI::Texture* Texture    = nullptr;
        ResourceState Before     = ResourceState::eUndefined;
        ResourceState After      = ResourceState::eUndefined;
        u32           BaseMip    = 0;
        u32           MipCount   = kAllSubresources;
        u32           BaseLayer  = 0;
        u32           LayerCount = kAllSubresources;
    };

    struct BufferBarrier
    {
        RHI::Buffer*  Buffer = nullptr;
        ResourceState Before = ResourceState::eUndefined;
        ResourceState After  = ResourceState::eUndefined;
    };

    struct TextureRegion
    {
        u32      MipLevel = 0;
        u32      Layer    = 0;
        /// Offset of the region in texels.
        u32      X = 0, Y = 0, Z = 0;
        /// Zero extent means the whole mip level.
        Extent3D Extent = {0, 0, 0};
    };

    class VT_RENDERER_RHI_API CommandList
    {
      public:
        CommandList(const CommandList&)            = delete;
        CommandList& operator=(const CommandList&) = delete;
        virtual ~CommandList()                     = default;

        inline QueueType Queue() const { return m_Queue; }

        //--------------------------------------------------------------------
        // Rendering ---------------------------------------------------------
        //--------------------------------------------------------------------

        /// Starts rendering into the attachments. Attachments must already
        /// be in eColorAttachment / eDepthStencilWrite (or Read) state; use
        /// Transition() or let the RenderGraph do it. Viewport and scissor
        /// are reset to the render area.
        virtual void BeginRendering(const RenderingDesc& desc) = 0;
        virtual void EndRendering()                            = 0;

        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetScissor(const Rect2D& scissor)     = 0;

        /// Binds a graphics or compute pipeline; BindGroup and PushConstants
        /// then target its bind point.
        virtual void BindPipeline(Pipeline& pipeline) = 0;
        virtual void BindVertexBuffer(u32 slot, Buffer& buffer,
                                      usize offset = 0)
            = 0;
        virtual void BindIndexBuffer(Buffer& buffer, IndexType type,
                                     usize offset = 0)
            = 0;
        /// Binds `group` to `set` of the currently bound pipeline.
        virtual void BindGroup(u32 set, RHI::BindGroup& group) = 0;
        virtual void PushConstants(ShaderStage stages, u32 offset, u32 size,
                                   const void* data)
            = 0;
        template <typename T>
        inline void PushConstants(ShaderStage stages, const T& value,
                                  u32 offset = 0)
        {
            PushConstants(stages, offset, sizeof(T), &value);
        }

        virtual void Draw(u32 vertexCount, u32 instanceCount = 1,
                          u32 firstVertex = 0, u32 firstInstance = 0)
            = 0;
        virtual void DrawIndexed(u32 indexCount, u32 instanceCount = 1,
                                 u32 firstIndex = 0, i32 vertexOffset = 0,
                                 u32 firstInstance = 0)
            = 0;
        virtual void DrawIndexedIndirect(Buffer& arguments, usize offset,
                                         u32 drawCount, u32 stride)
            = 0;

        /// Runs the bound compute pipeline over a grid of workgroups.
        /// Outside BeginRendering.
        virtual void Dispatch(u32 groupsX, u32 groupsY = 1, u32 groupsZ = 1) = 0;

        //--------------------------------------------------------------------
        // Transfer ----------------------------------------------------------
        //--------------------------------------------------------------------

        virtual void CopyBuffer(Buffer& source, usize sourceOffset,
                                Buffer& destination, usize destinationOffset,
                                usize size)
            = 0;
        /// Texture must be in eTransferDst state.
        virtual void CopyBufferToTexture(Buffer& source, usize sourceOffset,
                                         Texture&             destination,
                                         const TextureRegion& region)
            = 0;
        /// Fills mips 1..N from mip 0 by blitting. Mip 0 must be in
        /// eTransferDst state; the whole texture ends in `finalState`.
        virtual void GenerateMips(Texture&      texture,
                                  ResourceState finalState
                                  = ResourceState::eShaderRead)
            = 0;

        //--------------------------------------------------------------------
        // Synchronization ---------------------------------------------------
        //--------------------------------------------------------------------

        /// Explicit barriers. Updates the tracked state of whole-resource
        /// texture barriers.
        virtual void Barrier(Span<const TextureBarrier> textures,
                             Span<const BufferBarrier>  buffers = {})
            = 0;

        /// Moves `texture` from its tracked state to `after`; no-op if it is
        /// already there (except for writable states, which always need a
        /// barrier to order successive writes).
        inline void Transition(Texture& texture, ResourceState after)
        {
            ResourceState before = texture.TrackedState();
            if (before == after && after != ResourceState::eShaderWrite
                && after != ResourceState::eTransferDst
                && after != ResourceState::eGeneral)
                return;
            TextureBarrier barrier{&texture, before, after};
            Barrier(Span<const TextureBarrier>(&barrier, 1));
        }

        //--------------------------------------------------------------------
        // Debugging ---------------------------------------------------------
        //--------------------------------------------------------------------

        /// Named, colored region shown by GPU debuggers (RenderDoc, Nsight).
        virtual void BeginLabel(StringView name, ClearColor color = {1, 1, 1, 1})
            = 0;
        virtual void EndLabel() = 0;

        /// Named GPU timing zone (shown by Tracy with profiling enabled;
        /// otherwise free). Zones nest and must close in the same list.
        virtual void BeginGpuZone(StringView name) = 0;
        virtual void EndGpuZone()                  = 0;

      protected:
        explicit CommandList(QueueType queue)
            : m_Queue(queue)
        {
        }

        QueueType m_Queue;
    };

    /// RAII GPU timing zone: `ScopedGpuZone zone(cmd, "Shadows");`
    class ScopedGpuZone
    {
      public:
        inline ScopedGpuZone(CommandList& cmd, StringView name)
            : m_Cmd(cmd)
        {
            m_Cmd.BeginGpuZone(name);
        }
        inline ~ScopedGpuZone() { m_Cmd.EndGpuZone(); }

        ScopedGpuZone(const ScopedGpuZone&)            = delete;
        ScopedGpuZone& operator=(const ScopedGpuZone&) = delete;

      private:
        CommandList& m_Cmd;
    };

    /// RAII debug label: `ScopedLabel label(cmd, "Shadows");`
    class ScopedLabel
    {
      public:
        inline ScopedLabel(CommandList& cmd, StringView name,
                           ClearColor color = {1, 1, 1, 1})
            : m_Cmd(cmd)
        {
            m_Cmd.BeginLabel(name, color);
        }
        inline ~ScopedLabel() { m_Cmd.EndLabel(); }

        ScopedLabel(const ScopedLabel&)            = delete;
        ScopedLabel& operator=(const ScopedLabel&) = delete;

      private:
        CommandList& m_Cmd;
    };
} // namespace Vortex::RHI
