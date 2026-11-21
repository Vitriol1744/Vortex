/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
// Tracy's Vulkan header has C headers: before VulkanCommon.hpp's imports.
#ifdef TRACY_ENABLE
    #include <vulkan/vulkan.h>
    #include <tracy/TracyVulkan.hpp>
#endif
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommandList.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanPipeline.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanResources.hpp>

import Prism.Containers.Vector;

namespace Vortex::Vulkan
{
    using namespace RHI;

    namespace
    {
        u32 MipSize(u32 size, u32 mip)
        {
            u32 result = size >> mip;
            return result ? result : 1;
        }

        bool IsWholeResource(const TextureBarrier& barrier)
        {
            return barrier.BaseMip == 0 && barrier.BaseLayer == 0
                && barrier.MipCount == kAllSubresources
                && barrier.LayerCount == kAllSubresources;
        }
    } // namespace

    VulkanCommandList::VulkanCommandList(QueueType queue, vk::CommandBuffer buffer)
        : CommandList(queue)
        , m_Buffer(buffer)
    {
    }

    VulkanCommandList::~VulkanCommandList()
    {
        VtCoreAssertMsg(m_GpuZones.Empty(),
                        "Vulkan: Command list destroyed with open GPU zones");
    }

    void VulkanCommandList::Begin()
    {
        VtCoreAssertMsg(!m_Recording, "Vulkan: Command list already recording");
        vk::CommandBufferBeginInfo info{};
        info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        VkCall(m_Buffer.begin(&info));
        m_Recording = true;
        m_Rendering = false;
        m_Pipeline  = nullptr;
        m_Labels    = 0;
    }
    void VulkanCommandList::End()
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering,
                        "Vulkan: Command list ended inside BeginRendering");
        VtCoreAssertMsg(m_Labels == 0,
                        "Vulkan: Command list ended with an open label");
        VtCoreAssertMsg(m_GpuZones.Empty(),
                        "Vulkan: Command list ended with an open GPU zone");
        m_Buffer.end();
        m_Recording = false;
    }
    void VulkanCommandList::CheckRecording() const
    {
        VtCoreAssertMsg(m_Recording,
                        "Vulkan: Command list used outside of recording; get "
                        "one from Device::BeginCommandList");
    }

    //------------------------------------------------------------------------
    // Rendering -------------------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanCommandList::BeginRendering(const RenderingDesc& desc)
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering, "Vulkan: Nested BeginRendering");
        VtCoreAssertMsg(!desc.Colors.Empty() || desc.Depth.Texture,
                        "Vulkan: BeginRendering without attachments");

        if (!desc.DebugName.Empty()) BeginLabel(desc.DebugName, {0.4f, 0.7f, 1.0f, 1.0f});

        Extent2D extent{};
        Vector<vk::RenderingAttachmentInfo> colors;
        for (const auto& color : desc.Colors)
        {
            VtCoreAssertMsg(color.Texture, "Vulkan: Null color attachment");
            VtCoreAssertFormat(color.Texture->TrackedState()
                                   == ResourceState::eColorAttachment,
                               "Vulkan: Color attachment '{}' isn't in the "
                               "eColorAttachment state",
                               color.Texture->Desc().DebugName);
            auto* texture = static_cast<VulkanTexture*>(color.Texture.Raw());
            if (extent.Empty())
                extent = {MipSize(texture->Width(), color.MipLevel),
                          MipSize(texture->Height(), color.MipLevel)};

            vk::RenderingAttachmentInfo& info = colors.EmplaceBack();
            info.imageView   = texture->View(color.MipLevel, color.Layer);
            info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            info.loadOp      = ToVk(color.Load);
            info.storeOp     = ToVk(color.Store);
            info.clearValue.color.setFloat32(
                {color.Clear.R, color.Clear.G, color.Clear.B, color.Clear.A});
        }

        vk::RenderingAttachmentInfo depth{};
        bool                        hasStencil = false;
        if (desc.Depth.Texture)
        {
            ResourceState expected = desc.Depth.ReadOnly
                                       ? ResourceState::eDepthStencilRead
                                       : ResourceState::eDepthStencilWrite;
            VtCoreAssertFormat(desc.Depth.Texture->TrackedState() == expected,
                               "Vulkan: Depth attachment '{}' isn't in the "
                               "depth attachment state",
                               desc.Depth.Texture->Desc().DebugName);
            auto* texture = static_cast<VulkanTexture*>(desc.Depth.Texture.Raw());
            if (extent.Empty()) extent = {texture->Width(), texture->Height()};
            hasStencil = HasStencil(texture->Format());

            depth.imageView   = texture->View(0, 0);
            depth.imageLayout = desc.Depth.ReadOnly
                                  ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
                                  : vk::ImageLayout::eDepthStencilAttachmentOptimal;
            depth.loadOp  = ToVk(desc.Depth.Load);
            depth.storeOp = ToVk(desc.Depth.Store);
            depth.clearValue.depthStencil
                = vk::ClearDepthStencilValue(desc.Depth.Clear.Depth,
                                             desc.Depth.Clear.Stencil);
        }

        Rect2D area = desc.Area;
        if (area.Extent.Empty()) area = {{0, 0}, extent};

        vk::RenderingInfo info{};
        info.renderArea           = vk::Rect2D({area.Offset.X, area.Offset.Y},
                                               {area.Extent.Width, area.Extent.Height});
        info.layerCount           = 1;
        info.colorAttachmentCount = colors.Size();
        info.pColorAttachments    = colors.Raw();
        if (desc.Depth.Texture)
        {
            info.pDepthAttachment = &depth;
            if (hasStencil) info.pStencilAttachment = &depth;
        }
        m_Buffer.beginRendering(&info);
        m_Rendering = true;
        m_LabelOwnedByRendering = !desc.DebugName.Empty();

        SetViewport({static_cast<f32>(area.Offset.X),
                     static_cast<f32>(area.Offset.Y),
                     static_cast<f32>(area.Extent.Width),
                     static_cast<f32>(area.Extent.Height), 0.0f, 1.0f});
        SetScissor(area);
    }

    void VulkanCommandList::EndRendering()
    {
        CheckRecording();
        VtCoreAssertMsg(m_Rendering, "Vulkan: EndRendering without BeginRendering");
        m_Buffer.endRendering();
        m_Rendering = false;
        if (m_LabelOwnedByRendering) EndLabel();
        m_LabelOwnedByRendering = false;
    }

    void VulkanCommandList::SetViewport(const Viewport& viewport)
    {
        CheckRecording();
        vk::Viewport vkViewport(viewport.X, viewport.Y, viewport.Width,
                                viewport.Height, viewport.MinDepth,
                                viewport.MaxDepth);
        m_Buffer.setViewport(0, 1, &vkViewport);
    }
    void VulkanCommandList::SetScissor(const Rect2D& scissor)
    {
        CheckRecording();
        vk::Rect2D rect({scissor.Offset.X, scissor.Offset.Y},
                        {scissor.Extent.Width, scissor.Extent.Height});
        m_Buffer.setScissor(0, 1, &rect);
    }

    void VulkanCommandList::BindPipeline(Pipeline& pipeline)
    {
        CheckRecording();
        m_Pipeline = static_cast<VulkanPipeline*>(&pipeline);
        m_Buffer.bindPipeline(m_Pipeline->BindPoint(), m_Pipeline->Handle());
    }
    void VulkanCommandList::BindVertexBuffer(u32 slot, Buffer& buffer,
                                             usize offset)
    {
        CheckRecording();
        vk::Buffer     handle   = static_cast<VulkanBuffer&>(buffer).Handle();
        vk::DeviceSize vkOffset = offset;
        m_Buffer.bindVertexBuffers(slot, 1, &handle, &vkOffset);
    }
    void VulkanCommandList::BindIndexBuffer(Buffer& buffer, IndexType type,
                                            usize offset)
    {
        CheckRecording();
        m_Buffer.bindIndexBuffer(static_cast<VulkanBuffer&>(buffer).Handle(),
                                 offset, ToVk(type));
    }
    void VulkanCommandList::BindGroup(u32 set, RHI::BindGroup& group)
    {
        CheckRecording();
        VtCoreAssertMsg(m_Pipeline, "Vulkan: BindGroup before BindPipeline");
        vk::DescriptorSet handle = static_cast<VulkanBindGroup&>(group).Handle();
        m_Buffer.bindDescriptorSets(m_Pipeline->BindPoint(), m_Pipeline->Layout(),
                                    set, 1, &handle, 0, nullptr);
    }

    void VulkanCommandList::Dispatch(u32 groupsX, u32 groupsY, u32 groupsZ)
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering, "Vulkan: Dispatch inside rendering");
        VtCoreAssertMsg(m_Pipeline
                            && m_Pipeline->Type() == PipelineType::eCompute,
                        "Vulkan: Dispatch without a compute pipeline");
        m_Buffer.dispatch(groupsX, groupsY, groupsZ);
    }
    void VulkanCommandList::PushConstants(ShaderStage stages, u32 offset,
                                          u32 size, const void* data)
    {
        CheckRecording();
        VtCoreAssertMsg(m_Pipeline, "Vulkan: PushConstants before BindPipeline");
        VtCoreAssertMsg(m_Pipeline->PushConstantStages(),
                        "Vulkan: The pipeline's shader declares no push "
                        "constants");
        (void)stages;
        // Vulkan wants the stage flags of every range the update overlaps;
        // the shader has one range, so use all of its stages.
        m_Buffer.pushConstants(m_Pipeline->Layout(),
                               m_Pipeline->PushConstantStages(), offset, size,
                               data);
    }

    void VulkanCommandList::Draw(u32 vertexCount, u32 instanceCount,
                                 u32 firstVertex, u32 firstInstance)
    {
        CheckRecording();
        VtCoreAssertMsg(m_Rendering && m_Pipeline,
                        "Vulkan: Draw outside rendering or without a pipeline");
        m_Buffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }
    void VulkanCommandList::DrawIndexed(u32 indexCount, u32 instanceCount,
                                        u32 firstIndex, i32 vertexOffset,
                                        u32 firstInstance)
    {
        CheckRecording();
        VtCoreAssertMsg(m_Rendering && m_Pipeline,
                        "Vulkan: Draw outside rendering or without a pipeline");
        m_Buffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset,
                             firstInstance);
    }
    void VulkanCommandList::DrawIndexedIndirect(Buffer& arguments, usize offset,
                                                u32 drawCount, u32 stride)
    {
        CheckRecording();
        VtCoreAssertMsg(m_Rendering && m_Pipeline,
                        "Vulkan: Draw outside rendering or without a pipeline");
        m_Buffer.drawIndexedIndirect(
            static_cast<VulkanBuffer&>(arguments).Handle(), offset, drawCount,
            stride);
    }

    //------------------------------------------------------------------------
    // Transfer --------------------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanCommandList::CopyBuffer(Buffer& source, usize sourceOffset,
                                       Buffer& destination,
                                       usize destinationOffset, usize size)
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering, "Vulkan: Copy inside rendering");
        vk::BufferCopy region(sourceOffset, destinationOffset, size);
        m_Buffer.copyBuffer(static_cast<VulkanBuffer&>(source).Handle(),
                            static_cast<VulkanBuffer&>(destination).Handle(), 1,
                            &region);
    }

    void VulkanCommandList::CopyBufferToTexture(Buffer& source,
                                                usize   sourceOffset,
                                                Texture& destination,
                                                const TextureRegion& region)
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering, "Vulkan: Copy inside rendering");
        auto& texture = static_cast<VulkanTexture&>(destination);

        Extent3D extent = region.Extent;
        if (extent.Width == 0)
            extent = {MipSize(texture.Width(), region.MipLevel),
                      MipSize(texture.Height(), region.MipLevel),
                      MipSize(texture.Extent().Depth, region.MipLevel)};

        vk::BufferImageCopy copy{};
        copy.bufferOffset                    = sourceOffset;
        copy.imageSubresource.aspectMask     = texture.Aspect();
        copy.imageSubresource.mipLevel       = region.MipLevel;
        copy.imageSubresource.baseArrayLayer = region.Layer;
        copy.imageSubresource.layerCount     = 1;
        copy.imageOffset = vk::Offset3D(static_cast<i32>(region.X),
                                        static_cast<i32>(region.Y),
                                        static_cast<i32>(region.Z));
        copy.imageExtent = vk::Extent3D(extent.Width, extent.Height,
                                        extent.Depth);
        m_Buffer.copyBufferToImage(static_cast<VulkanBuffer&>(source).Handle(),
                                   texture.Handle(),
                                   vk::ImageLayout::eTransferDstOptimal, 1,
                                   &copy);
    }

    void VulkanCommandList::GenerateMips(Texture&      texture,
                                         ResourceState finalState)
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering, "Vulkan: GenerateMips inside rendering");
        RecordGenerateMips(m_Buffer, static_cast<VulkanTexture&>(texture),
                           finalState);
    }

    void VulkanCommandList::RecordGenerateMips(vk::CommandBuffer cmd,
                                               VulkanTexture&    texture,
                                               ResourceState     finalState)
    {
        u32 mips   = texture.MipLevels();
        u32 layers = texture.Layers();
        VtCoreAssertMsg(texture.TrackedState() == ResourceState::eTransferDst,
                        "Vulkan: GenerateMips needs the texture in "
                        "eTransferDst state");

        for (u32 mip = 1; mip < mips; ++mip)
        {
            // Mip-1 becomes the blit source; mip is already TransferDst.
            TextureBarrier toSource{&texture, ResourceState::eTransferDst,
                                    ResourceState::eTransferSrc, mip - 1, 1,
                                    0, layers};
            RecordBarrier(cmd, Span<const TextureBarrier>(&toSource, 1), {});

            vk::ImageBlit blit{};
            blit.srcOffsets[1] = vk::Offset3D(
                static_cast<i32>(MipSize(texture.Width(), mip - 1)),
                static_cast<i32>(MipSize(texture.Height(), mip - 1)), 1);
            blit.srcSubresource = vk::ImageSubresourceLayers(
                texture.Aspect(), mip - 1, 0, layers);
            blit.dstOffsets[1] = vk::Offset3D(
                static_cast<i32>(MipSize(texture.Width(), mip)),
                static_cast<i32>(MipSize(texture.Height(), mip)), 1);
            blit.dstSubresource
                = vk::ImageSubresourceLayers(texture.Aspect(), mip, 0, layers);
            cmd.blitImage(texture.Handle(), vk::ImageLayout::eTransferSrcOptimal,
                          texture.Handle(), vk::ImageLayout::eTransferDstOptimal,
                          1, &blit, vk::Filter::eLinear);
        }

        // Mips 0..N-2 are blit sources, the last one is a destination.
        TextureBarrier toFinal[2] = {
            {&texture, ResourceState::eTransferSrc, finalState, 0, mips - 1, 0,
             layers},
            {&texture, ResourceState::eTransferDst, finalState, mips - 1, 1, 0,
             layers},
        };
        if (mips > 1)
            RecordBarrier(cmd, Span<const TextureBarrier>(toFinal, 2), {});
        else RecordBarrier(cmd, Span<const TextureBarrier>(&toFinal[1], 1), {});
        texture.SetTrackedState(finalState);
    }

    //------------------------------------------------------------------------
    // Barriers --------------------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanCommandList::Barrier(Span<const TextureBarrier> textures,
                                    Span<const BufferBarrier>  buffers)
    {
        CheckRecording();
        VtCoreAssertMsg(!m_Rendering, "Vulkan: Barrier inside rendering");
        RecordBarrier(m_Buffer, textures, buffers);
    }

    void VulkanCommandList::RecordBarrier(vk::CommandBuffer          cmd,
                                          Span<const TextureBarrier> textures,
                                          Span<const BufferBarrier>  buffers)
    {
        Vector<vk::ImageMemoryBarrier2>  imageBarriers;
        Vector<vk::BufferMemoryBarrier2> bufferBarriers;

        for (const auto& barrier : textures)
        {
            VtCoreAssertMsg(barrier.Texture, "Vulkan: Barrier on a null texture");
            auto*     texture = static_cast<VulkanTexture*>(barrier.Texture);
            StateInfo src     = GetStateInfo(barrier.Before, true);
            StateInfo dst     = GetStateInfo(barrier.After, false);

            vk::ImageMemoryBarrier2& vkBarrier = imageBarriers.EmplaceBack();
            vkBarrier.srcStageMask  = src.Stages;
            vkBarrier.srcAccessMask = src.Access;
            vkBarrier.dstStageMask  = dst.Stages;
            vkBarrier.dstAccessMask = dst.Access;
            vkBarrier.oldLayout     = src.Layout;
            vkBarrier.newLayout     = dst.Layout;
            vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vkBarrier.image               = texture->Handle();
            vkBarrier.subresourceRange.aspectMask   = texture->Aspect();
            vkBarrier.subresourceRange.baseMipLevel = barrier.BaseMip;
            vkBarrier.subresourceRange.levelCount
                = barrier.MipCount == kAllSubresources ? VK_REMAINING_MIP_LEVELS
                                                       : barrier.MipCount;
            vkBarrier.subresourceRange.baseArrayLayer = barrier.BaseLayer;
            vkBarrier.subresourceRange.layerCount
                = barrier.LayerCount == kAllSubresources
                    ? VK_REMAINING_ARRAY_LAYERS
                    : barrier.LayerCount;

            if (IsWholeResource(barrier)) texture->SetTrackedState(barrier.After);
        }

        for (const auto& barrier : buffers)
        {
            VtCoreAssertMsg(barrier.Buffer, "Vulkan: Barrier on a null buffer");
            StateInfo src = GetStateInfo(barrier.Before, true);
            StateInfo dst = GetStateInfo(barrier.After, false);

            vk::BufferMemoryBarrier2& vkBarrier = bufferBarriers.EmplaceBack();
            vkBarrier.srcStageMask  = src.Stages;
            vkBarrier.srcAccessMask = src.Access;
            vkBarrier.dstStageMask  = dst.Stages;
            vkBarrier.dstAccessMask = dst.Access;
            vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            vkBarrier.buffer = static_cast<VulkanBuffer*>(barrier.Buffer)->Handle();
            vkBarrier.offset = 0;
            vkBarrier.size   = VK_WHOLE_SIZE;
        }

        if (imageBarriers.Empty() && bufferBarriers.Empty()) return;
        vk::DependencyInfo dependency{};
        dependency.imageMemoryBarrierCount  = imageBarriers.Size();
        dependency.pImageMemoryBarriers     = imageBarriers.Raw();
        dependency.bufferMemoryBarrierCount = bufferBarriers.Size();
        dependency.pBufferMemoryBarriers    = bufferBarriers.Raw();
        cmd.pipelineBarrier2(&dependency);
    }

    //------------------------------------------------------------------------
    // Labels ----------------------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanCommandList::BeginLabel(StringView name, ClearColor color)
    {
        CheckRecording();
        ++m_Labels;
        if (!VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdBeginDebugUtilsLabelEXT) return;
        String               terminated(name);
        vk::DebugUtilsLabelEXT label{};
        label.pLabelName = terminated.Raw();
        label.color      = std::array<f32, 4>{color.R, color.G, color.B, color.A};
        m_Buffer.beginDebugUtilsLabelEXT(&label);
    }
    void VulkanCommandList::EndLabel()
    {
        CheckRecording();
        VtCoreAssertMsg(m_Labels > 0, "Vulkan: EndLabel without BeginLabel");
        --m_Labels;
        if (!VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdEndDebugUtilsLabelEXT) return;
        m_Buffer.endDebugUtilsLabelEXT();
    }

    //------------------------------------------------------------------------
    // GPU zones -------------------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanCommandList::BeginGpuZone(StringView name)
    {
        CheckRecording();
#ifdef TRACY_ENABLE
        auto* context = static_cast<tracy::VkCtx*>(VulkanDevice::Get().TracyContext());
        if (context)
        {
            static constexpr const char kFile[]     = __FILE__;
            static constexpr const char kFunction[] = "GPU zone";
            m_GpuZones.PushBack(new tracy::VkCtxScope(
                context, __LINE__, kFile, sizeof(kFile) - 1, kFunction,
                sizeof(kFunction) - 1, name.Raw(), name.Size(), m_Buffer, true));
            return;
        }
#endif
        (void)name;
        m_GpuZones.PushBack(nullptr);
    }
    void VulkanCommandList::EndGpuZone()
    {
        CheckRecording();
        VtCoreAssertMsg(!m_GpuZones.Empty(), "Vulkan: EndGpuZone without BeginGpuZone");
#ifdef TRACY_ENABLE
        delete static_cast<tracy::VkCtxScope*>(m_GpuZones.Back());
#endif
        m_GpuZones.PopBack();
    }
} // namespace Vortex::Vulkan
