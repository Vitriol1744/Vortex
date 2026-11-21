/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Core/Types.hpp>

#include <cstring>

import Prism.Containers.Span;
import Prism.String.Formatter;
import Vortex.Renderer;
import Vortex.Renderer.RenderGraph;

namespace Vortex
{
    using namespace Details;

    namespace
    {
        bool IsWriteState(RHI::ResourceState state)
        {
            switch (state)
            {
                case RHI::ResourceState::eGeneral:
                case RHI::ResourceState::eShaderWrite:
                case RHI::ResourceState::eColorAttachment:
                case RHI::ResourceState::eDepthStencilWrite:
                case RHI::ResourceState::eTransferDst: return true;
                default: return false;
            }
        }

        bool SameDesc(const RHI::TextureDesc& a, const RHI::TextureDesc& b)
        {
            return a.Type == b.Type && a.Format == b.Format && a.Extent == b.Extent
                && a.MipLevels == b.MipLevels && a.Layers == b.Layers
                && a.Samples == b.Samples && a.Usage == b.Usage;
        }
    } // namespace

    //------------------------------------------------------------------------
    // RGContext -------------------------------------------------------------
    //------------------------------------------------------------------------

    ::Ref<RHI::Texture> RGContext::Get(RGTexture texture) const
    {
        VtCoreAssertMsg(texture.Valid() && texture.Index < m_Graph.m_Resources.Size()
                            && m_Graph.m_Resources[texture.Index].Texture,
                        "RenderGraph: Invalid texture handle");
        return m_Graph.m_Resources[texture.Index].TextureRef;
    }
    ::Ref<RHI::Buffer> RGContext::Get(RGBuffer buffer) const
    {
        VtCoreAssertMsg(buffer.Valid() && buffer.Index < m_Graph.m_Resources.Size()
                            && !m_Graph.m_Resources[buffer.Index].Texture,
                        "RenderGraph: Invalid buffer handle");
        return m_Graph.m_Resources[buffer.Index].BufferRef;
    }

    //------------------------------------------------------------------------
    // RGBuilder -------------------------------------------------------------
    //------------------------------------------------------------------------

    void RGBuilder::Access(u32 resource, bool texture, RHI::ResourceState state,
                           bool read, bool write)
    {
        VtCoreAssertFormat(resource < m_Graph.m_Resources.Size()
                               && m_Graph.m_Resources[resource].Texture == texture,
                           "RenderGraph: Pass '{}' uses an invalid {} handle",
                           m_Pass.Name, texture ? "texture" : "buffer");
        for (auto& access : m_Pass.Accesses)
        {
            if (access.Resource != resource || access.Texture != texture) continue;
            VtCoreAssertFormat(access.State == state,
                               "RenderGraph: Pass '{}' uses '{}' in two states",
                               m_Pass.Name, m_Graph.m_Resources[resource].Name);
            access.Read |= read;
            access.Write |= write;
            return;
        }
        m_Pass.Accesses.PushBack({resource, texture, state, write, read});
    }

    RGTexture RGBuilder::CreateTexture(const RHI::TextureDesc& desc)
    {
        RenderGraph::Resource resource{};
        resource.Name    = desc.DebugName.Empty()
                             ? Prism::Format("{} transient", m_Pass.Name)
                             : desc.DebugName;
        resource.Texture = true;
        resource.Desc    = desc;
        m_Graph.m_Resources.PushBack(Move(resource));
        return {static_cast<u32>(m_Graph.m_Resources.Size() - 1)};
    }

    RGTexture RGBuilder::Read(RGTexture texture, RHI::ResourceState state)
    {
        Access(texture.Index, true, state, true, IsWriteState(state));
        return texture;
    }
    RGTexture RGBuilder::Write(RGTexture texture, RHI::ResourceState state)
    {
        Access(texture.Index, true, state, false, true);
        m_Graph.m_Resources[texture.Index].Desc.Usage
            |= state == RHI::ResourceState::eShaderWrite ? RHI::TextureUsage::eStorage
                                                         : RHI::TextureUsage::eNone;
        return texture;
    }
    RGTexture RGBuilder::WriteColor(RGTexture texture, RHI::LoadOp load,
                                    RHI::ClearColor clear)
    {
        // Loading keeps the old contents: that's a read of the previous
        // writer too.
        Access(texture.Index, true, RHI::ResourceState::eColorAttachment,
               load == RHI::LoadOp::eLoad, true);
        RGAttachment attachment{};
        attachment.Resource   = texture.Index;
        attachment.Load       = load;
        attachment.ClearColor = clear;
        m_Pass.Attachments.PushBack(attachment);
        m_Graph.m_Resources[texture.Index].Desc.Usage |= RHI::TextureUsage::eColorAttachment;
        return texture;
    }
    RGTexture RGBuilder::WriteDepth(RGTexture texture, RHI::LoadOp load,
                                    RHI::ClearDepthStencil clear)
    {
        Access(texture.Index, true, RHI::ResourceState::eDepthStencilWrite,
               load == RHI::LoadOp::eLoad, true);
        RGAttachment attachment{};
        attachment.Resource   = texture.Index;
        attachment.Depth      = true;
        attachment.Load       = load;
        attachment.ClearDepth = clear;
        m_Pass.Attachments.PushBack(attachment);
        m_Graph.m_Resources[texture.Index].Desc.Usage
            |= RHI::TextureUsage::eDepthStencilAttachment;
        return texture;
    }
    RGTexture RGBuilder::ReadDepth(RGTexture texture)
    {
        Access(texture.Index, true, RHI::ResourceState::eDepthStencilRead, true, false);
        RGAttachment attachment{};
        attachment.Resource = texture.Index;
        attachment.Depth    = true;
        attachment.ReadOnly = true;
        attachment.Load     = RHI::LoadOp::eLoad;
        m_Pass.Attachments.PushBack(attachment);
        return texture;
    }
    RGBuffer RGBuilder::Read(RGBuffer buffer, RHI::ResourceState state)
    {
        Access(buffer.Index, false, state, true, IsWriteState(state));
        return buffer;
    }
    RGBuffer RGBuilder::Write(RGBuffer buffer, RHI::ResourceState state)
    {
        Access(buffer.Index, false, state, false, true);
        return buffer;
    }
    void RGBuilder::SideEffect() { m_Pass.SideEffect = true; }

    //------------------------------------------------------------------------
    // RenderGraph -----------------------------------------------------------
    //------------------------------------------------------------------------

    RGTexture RenderGraph::Import(::Ref<RHI::Texture> texture,
                                  RHI::ResourceState  finalState)
    {
        VtCoreAssertMsg(texture, "RenderGraph: Importing a null texture");
        Resource resource{};
        resource.Name       = texture->Desc().DebugName;
        resource.Texture    = true;
        resource.Imported   = true;
        resource.Desc       = texture->Desc();
        resource.TextureRef = texture;
        resource.FinalState = finalState;
        m_Resources.PushBack(Move(resource));
        return {static_cast<u32>(m_Resources.Size() - 1)};
    }
    RGBuffer RenderGraph::Import(::Ref<RHI::Buffer> buffer)
    {
        VtCoreAssertMsg(buffer, "RenderGraph: Importing a null buffer");
        Resource resource{};
        resource.Name      = buffer->Desc().DebugName;
        resource.Texture   = false;
        resource.Imported  = true;
        resource.BufferRef = buffer;
        m_Resources.PushBack(Move(resource));
        return {static_cast<u32>(m_Resources.Size() - 1)};
    }

    void RenderGraph::Cull()
    {
        usize passCount     = m_Passes.Size();
        usize resourceCount = m_Resources.Size();

        // For each pass and each resource it reads: the pass that last wrote
        // that resource before it (its producer).
        Vector<Vector<u32>> producers(passCount);
        Vector<i64>         lastWriter(resourceCount);
        for (auto& writer : lastWriter) writer = -1;
        for (u32 p = 0; p < passCount; ++p)
        {
            for (const auto& access : m_Passes[p]->Accesses)
            {
                if (access.Read && lastWriter[access.Resource] >= 0)
                    producers[p].PushBack(static_cast<u32>(lastWriter[access.Resource]));
            }
            for (const auto& access : m_Passes[p]->Accesses)
                if (access.Write) lastWriter[access.Resource] = p;
        }

        // Roots: side effects, and the final writer of each imported resource.
        Vector<bool> live(passCount);
        for (u32 p = 0; p < passCount; ++p) live[p] = m_Passes[p]->SideEffect;
        for (u32 r = 0; r < resourceCount; ++r)
            if (m_Resources[r].Imported && lastWriter[r] >= 0) live[lastWriter[r]] = true;

        // Walk back: a live pass keeps alive whoever produced its inputs.
        for (usize p = passCount; p-- > 0;)
        {
            if (!live[p]) continue;
            for (u32 producer : producers[p]) live[producer] = true;
        }
        for (u32 p = 0; p < passCount; ++p) m_Passes[p]->Culled = !live[p];
    }

    usize RenderGraph::CulledCount() const
    {
        usize culled = 0;
        for (const auto& pass : m_Passes) culled += pass->Culled ? 1 : 0;
        return culled;
    }

    void RenderGraph::Execute(RHI::CommandList& cmd, RGTransientPool& pool)
    {
        VtProfileFunction();
        Cull();

        // Transients used by a live pass.
        for (const auto& pass : m_Passes)
        {
            if (pass->Culled) continue;
            for (const auto& access : pass->Accesses)
            {
                Resource& resource = m_Resources[access.Resource];
                if (!resource.Texture || resource.Imported || resource.TextureRef) continue;
                resource.TextureRef = pool.Acquire(resource.Desc);
            }
        }

        RGContext context(*this, cmd);
        for (const auto& pass : m_Passes)
        {
            if (pass->Culled) continue;
            VtProfileScopeDynamic(pass->Name.Raw());
            RHI::ScopedLabel   label(cmd, pass->Name);
            RHI::ScopedGpuZone zone(cmd, pass->Name);

            // Barriers into the declared states.
            for (const auto& access : pass->Accesses)
            {
                Resource& resource = m_Resources[access.Resource];
                if (resource.Texture)
                {
                    cmd.Transition(*resource.TextureRef, access.State);
                    continue;
                }
                bool needed = resource.BufferState != access.State || access.Write;
                if (needed && resource.BufferState != RHI::ResourceState::eUndefined)
                {
                    RHI::BufferBarrier barrier{resource.BufferRef.Raw(),
                                               resource.BufferState, access.State};
                    cmd.Barrier({}, Span<const RHI::BufferBarrier>(&barrier, 1));
                }
                resource.BufferState = access.State;
            }

            bool rendering = !pass->Attachments.Empty();
            if (rendering)
            {
                RHI::RenderingDesc desc{};
                for (const auto& attachment : pass->Attachments)
                {
                    auto texture = m_Resources[attachment.Resource].TextureRef;
                    if (attachment.Depth)
                    {
                        desc.Depth.Texture  = texture;
                        desc.Depth.Load     = attachment.Load;
                        desc.Depth.Clear    = attachment.ClearDepth;
                        desc.Depth.ReadOnly = attachment.ReadOnly;
                        continue;
                    }
                    RHI::ColorAttachment& color = desc.Colors.EmplaceBack();
                    color.Texture               = texture;
                    color.Load                  = attachment.Load;
                    color.Clear                 = attachment.ClearColor;
                }
                cmd.BeginRendering(desc);
            }
            pass->Execute(context);
            if (rendering) cmd.EndRendering();
        }

        // Hand imported textures back in their final states.
        for (auto& resource : m_Resources)
            if (resource.Texture && resource.Imported && resource.TextureRef)
                cmd.Transition(*resource.TextureRef, resource.FinalState);
    }

    String RenderGraph::Graphviz() const
    {
        String dot = "digraph RenderGraph {\n  rankdir=LR;\n";
        for (usize r = 0; r < m_Resources.Size(); ++r)
            dot += Prism::Format("  r{} [shape=box, label=\"{}{}\"];\n", r,
                                 m_Resources[r].Name,
                                 m_Resources[r].Imported ? " (imported)" : "");
        for (usize p = 0; p < m_Passes.Size(); ++p)
        {
            const auto& pass = *m_Passes[p];
            dot += Prism::Format("  p{} [shape=ellipse, label=\"{}\"{}];\n", p,
                                 pass.Name,
                                 pass.Culled ? ", style=dashed, color=gray" : "");
            for (const auto& access : pass.Accesses)
            {
                if (access.Read) dot += Prism::Format("  r{} -> p{};\n", access.Resource, p);
                if (access.Write) dot += Prism::Format("  p{} -> r{};\n", p, access.Resource);
            }
        }
        dot += "}\n";
        return dot;
    }

    //------------------------------------------------------------------------
    // RGTransientPool -------------------------------------------------------
    //------------------------------------------------------------------------

    ::Ref<RHI::Texture> RGTransientPool::Acquire(const RHI::TextureDesc& desc)
    {
        for (auto& entry : m_Entries)
        {
            if (entry.InUse || !SameDesc(entry.Desc, desc)) continue;
            entry.InUse  = true;
            entry.Unused = 0;
            return entry.Texture;
        }
        Entry& entry  = m_Entries.EmplaceBack();
        entry.Desc    = desc;
        entry.Texture = Renderer::Device().CreateTexture(desc);
        entry.InUse   = true;
        return entry.Texture;
    }

    void RGTransientPool::EndFrame()
    {
        constexpr u32 kKeepFrames = 8;
        for (usize i = 0; i < m_Entries.Size();)
        {
            Entry& entry = m_Entries[i];
            if (!entry.InUse && ++entry.Unused > kKeepFrames)
            {
                m_Entries[i] = Move(m_Entries.Back());
                m_Entries.PopBack();
                continue;
            }
            entry.InUse = false;
            ++i;
        }
    }
} // namespace Vortex
