/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

import Prism.Containers.Span;
import Prism.Containers.Vector;

namespace Vortex::Vulkan
{
    class VulkanPipeline;
    class VulkanTexture;

    class VulkanCommandList final : public RHI::CommandList
    {
      public:
        VulkanCommandList(RHI::QueueType queue, vk::CommandBuffer buffer);
        ~VulkanCommandList() override;

        /// Starts recording; resets per-recording state.
        void                     Begin();
        void                     End();
        inline vk::CommandBuffer Handle() const { return m_Buffer; }
        inline bool              Recording() const { return m_Recording; }

        void BeginRendering(const RHI::RenderingDesc& desc) override;
        void EndRendering() override;
        void SetViewport(const RHI::Viewport& viewport) override;
        void SetScissor(const RHI::Rect2D& scissor) override;
        void BindPipeline(RHI::Pipeline& pipeline) override;
        void BindVertexBuffer(u32 slot, RHI::Buffer& buffer,
                              usize offset) override;
        void BindIndexBuffer(RHI::Buffer& buffer, RHI::IndexType type,
                             usize offset) override;
        void BindGroup(u32 set, RHI::BindGroup& group) override;
        void Dispatch(u32 groupsX, u32 groupsY, u32 groupsZ) override;
        void PushConstants(RHI::ShaderStage stages, u32 offset, u32 size,
                           const void* data) override;
        void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex,
                  u32 firstInstance) override;
        void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex,
                         i32 vertexOffset, u32 firstInstance) override;
        void DrawIndexedIndirect(RHI::Buffer& arguments, usize offset,
                                 u32 drawCount, u32 stride) override;
        void CopyBuffer(RHI::Buffer& source, usize sourceOffset,
                        RHI::Buffer& destination, usize destinationOffset,
                        usize size) override;
        void CopyBufferToTexture(RHI::Buffer& source, usize sourceOffset,
                                 RHI::Texture&             destination,
                                 const RHI::TextureRegion& region) override;
        void GenerateMips(RHI::Texture&      texture,
                          RHI::ResourceState finalState) override;
        void Barrier(Span<const RHI::TextureBarrier> textures,
                     Span<const RHI::BufferBarrier>  buffers) override;
        void BeginLabel(StringView name, RHI::ClearColor color) override;
        void EndLabel() override;
        void BeginGpuZone(StringView name) override;
        void EndGpuZone() override;

        /// Records the commands on a raw command buffer; the device uses these
        /// for its upload stream, which has no VulkanCommandList.
        static void RecordGenerateMips(vk::CommandBuffer cmd,
                                       VulkanTexture&    texture,
                                       RHI::ResourceState finalState);
        static void RecordBarrier(vk::CommandBuffer               cmd,
                                  Span<const RHI::TextureBarrier> textures,
                                  Span<const RHI::BufferBarrier>  buffers);

      private:
        vk::CommandBuffer m_Buffer    = VK_NULL_HANDLE;
        VulkanPipeline*   m_Pipeline  = nullptr;
        bool              m_Recording = false;
        bool              m_Rendering = false;
        i32               m_Labels    = 0;
        /// BeginRendering opened a label for RenderingDesc::DebugName.
        bool              m_LabelOwnedByRendering = false;
        /// Open Tracy GPU zones (tracy::VkCtxScope*), innermost last.
        Vector<void*>     m_GpuZones;

        void              CheckRecording() const;
    };
} // namespace Vortex::Vulkan
