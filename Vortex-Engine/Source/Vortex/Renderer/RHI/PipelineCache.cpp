/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Types.hpp>

#include <cstring>
#include <mutex>

module Vortex.RHI.PipelineCache;


namespace Vortex::RHI
{
    namespace
    {
        /// FNV-1a over the fields one by one (never over padding).
        struct Hasher
        {
            usize Value = 0xcbf29ce484222325ull;

            template <typename T>
            void Add(const T& value)
            {
                u8 bytes[sizeof(T)];
                std::memcpy(bytes, &value, sizeof(T));
                for (u8 byte : bytes)
                {
                    Value ^= byte;
                    Value *= 0x100000001b3ull;
                }
            }
        };

        /// Blend state of color attachment `i`; missing entries are "off".
        BlendAttachment BlendOf(const GraphicsPipelineDesc& desc, usize i)
        {
            return i < desc.Blend.Size() ? desc.Blend[i] : BlendAttachment{};
        }

        bool SameBlend(const BlendAttachment& lhs, const BlendAttachment& rhs)
        {
            // Factors don't matter while blending is off.
            if (!lhs.Enable && !rhs.Enable) return lhs.WriteMask == rhs.WriteMask;
            return lhs.Enable == rhs.Enable && lhs.SrcColor == rhs.SrcColor
                && lhs.DstColor == rhs.DstColor && lhs.ColorOp == rhs.ColorOp
                && lhs.SrcAlpha == rhs.SrcAlpha && lhs.DstAlpha == rhs.DstAlpha
                && lhs.AlphaOp == rhs.AlphaOp && lhs.WriteMask == rhs.WriteMask;
        }

        bool SameBinding(const VertexBinding& lhs, const VertexBinding& rhs)
        {
            if (lhs.Stride != rhs.Stride || lhs.PerInstance != rhs.PerInstance
                || lhs.Attributes.Size() != rhs.Attributes.Size())
                return false;
            for (usize i = 0; i < lhs.Attributes.Size(); ++i)
            {
                const auto& a = lhs.Attributes[i];
                const auto& b = rhs.Attributes[i];
                if (a.Location != b.Location || a.Format != b.Format
                    || a.Offset != b.Offset)
                    return false;
            }
            return true;
        }
    } // namespace

    usize HashOf(const GraphicsPipelineDesc& desc)
    {
        Hasher hash;
        hash.Add(PipelineType::eGraphics);
        hash.Add(desc.Shader.Raw());
        hash.Add(desc.VertexBindings.Size());
        for (const auto& binding : desc.VertexBindings)
        {
            hash.Add(binding.Stride);
            hash.Add(binding.PerInstance);
            for (const auto& attribute : binding.Attributes)
            {
                hash.Add(attribute.Location);
                hash.Add(attribute.Format);
                hash.Add(attribute.Offset);
            }
        }
        hash.Add(desc.Topology);
        hash.Add(desc.Raster.Polygon);
        hash.Add(desc.Raster.Cull);
        hash.Add(desc.Raster.Front);
        hash.Add(desc.Raster.DepthClamp);
        hash.Add(desc.Raster.LineWidth);
        hash.Add(desc.DepthStencil.DepthTest);
        hash.Add(desc.DepthStencil.DepthWrite);
        hash.Add(desc.DepthStencil.DepthCompare);
        hash.Add(desc.ColorFormats.Size());
        for (usize i = 0; i < desc.ColorFormats.Size(); ++i)
        {
            hash.Add(desc.ColorFormats[i]);
            // Only what SameBlend compares, so equal descs hash equally.
            BlendAttachment blend = BlendOf(desc, i);
            hash.Add(blend.Enable);
            hash.Add(blend.WriteMask);
            if (!blend.Enable) continue;
            hash.Add(blend.SrcColor);
            hash.Add(blend.DstColor);
            hash.Add(blend.ColorOp);
            hash.Add(blend.SrcAlpha);
            hash.Add(blend.DstAlpha);
            hash.Add(blend.AlphaOp);
        }
        hash.Add(desc.DepthFormat);
        hash.Add(desc.Samples);
        return hash.Value;
    }

    usize HashOf(const ComputePipelineDesc& desc)
    {
        Hasher hash;
        hash.Add(PipelineType::eCompute);
        hash.Add(desc.Shader.Raw());
        return hash.Value;
    }

    bool Equivalent(const GraphicsPipelineDesc& lhs,
                    const GraphicsPipelineDesc& rhs)
    {
        if (lhs.Shader.Raw() != rhs.Shader.Raw() || lhs.Topology != rhs.Topology
            || lhs.DepthFormat != rhs.DepthFormat || lhs.Samples != rhs.Samples
            || lhs.VertexBindings.Size() != rhs.VertexBindings.Size()
            || lhs.ColorFormats.Size() != rhs.ColorFormats.Size())
            return false;

        const auto &ra = lhs.Raster, &rb = rhs.Raster;
        if (ra.Polygon != rb.Polygon || ra.Cull != rb.Cull
            || ra.Front != rb.Front || ra.DepthClamp != rb.DepthClamp
            || ra.LineWidth != rb.LineWidth)
            return false;

        const auto &da = lhs.DepthStencil, &db = rhs.DepthStencil;
        if (da.DepthTest != db.DepthTest || da.DepthWrite != db.DepthWrite
            || da.DepthCompare != db.DepthCompare)
            return false;

        for (usize i = 0; i < lhs.VertexBindings.Size(); ++i)
            if (!SameBinding(lhs.VertexBindings[i], rhs.VertexBindings[i]))
                return false;
        for (usize i = 0; i < lhs.ColorFormats.Size(); ++i)
            if (lhs.ColorFormats[i] != rhs.ColorFormats[i]
                || !SameBlend(BlendOf(lhs, i), BlendOf(rhs, i)))
                return false;
        return true;
    }

    PipelineCache::PipelineCache(Device& device)
        : m_Device(device)
    {
    }
    PipelineCache::~PipelineCache() { Clear(); }

    ::Ref<Pipeline> PipelineCache::Find(usize hash, PipelineType type,
                                        const void* desc)
    {
        auto it = m_Entries.Find(hash);
        if (it == m_Entries.end()) return nullptr;
        for (auto& entry : it->Value)
        {
            const Pipeline& pipeline = *entry.Pipeline;
            if (pipeline.Type() != type) continue;
            bool match
                = type == PipelineType::eCompute
                    ? pipeline.Shader().Raw()
                          == static_cast<const ComputePipelineDesc*>(desc)
                                 ->Shader.Raw()
                    : Equivalent(pipeline.Desc(),
                                 *static_cast<const GraphicsPipelineDesc*>(desc));
            if (!match) continue;
            entry.LastUsed = m_Frame;
            return entry.Pipeline;
        }
        return nullptr;
    }

    ::Ref<Pipeline> PipelineCache::Insert(usize hash, PipelineType type,
                                          const void* desc,
                                          ::Ref<Pipeline> created)
    {
        // Another thread may have made the same pipeline meanwhile: keep
        // theirs, so every caller ends up with one object.
        if (auto existing = Find(hash, type, desc)) return existing;
        m_Entries[hash].PushBack({created, m_Frame});
        ++m_Size;
        ++m_Created;
        return created;
    }

    ::Ref<Pipeline> PipelineCache::Get(const GraphicsPipelineDesc& desc)
    {
        if (!desc.Shader) return nullptr;
        usize hash = HashOf(desc);
        {
            std::lock_guard lock(m_Mutex);
            if (auto pipeline = Find(hash, PipelineType::eGraphics, &desc))
                return pipeline;
        }
        // Compiling takes long: don't hold up lookups of cached pipelines.
        auto created = m_Device.CreatePipeline(desc);
        VtCoreAssertFormat(created, "PipelineCache: Creating '{}' failed",
                           desc.DebugName);
        std::lock_guard lock(m_Mutex);
        return Insert(hash, PipelineType::eGraphics, &desc, Move(created));
    }

    ::Ref<Pipeline> PipelineCache::Get(const ComputePipelineDesc& desc)
    {
        if (!desc.Shader) return nullptr;
        usize hash = HashOf(desc);
        {
            std::lock_guard lock(m_Mutex);
            if (auto pipeline = Find(hash, PipelineType::eCompute, &desc))
                return pipeline;
        }
        auto created = m_Device.CreateComputePipeline(desc);
        VtCoreAssertFormat(created, "PipelineCache: Creating '{}' failed",
                           desc.DebugName);
        std::lock_guard lock(m_Mutex);
        return Insert(hash, PipelineType::eCompute, &desc, Move(created));
    }

    void PipelineCache::Invalidate(const Shader& shader)
    {
        std::lock_guard lock(m_Mutex);
        for (auto& [hash, entries] : m_Entries)
        {
            for (usize i = 0; i < entries.Size();)
            {
                if (entries[i].Pipeline->Shader().Raw() != &shader)
                {
                    ++i;
                    continue;
                }
                if (i + 1 != entries.Size()) entries[i] = Move(entries.Back());
                entries.PopBack();
                --m_Size;
            }
        }
    }

    void PipelineCache::Collect(u64 frame, u64 maxAge)
    {
        std::lock_guard lock(m_Mutex);
        m_Frame = frame;
        if (frame <= maxAge) return;
        u64 oldest = frame - maxAge;

        Vector<usize> empty;
        for (auto& [hash, entries] : m_Entries)
        {
            for (usize i = 0; i < entries.Size();)
            {
                if (entries[i].LastUsed >= oldest)
                {
                    ++i;
                    continue;
                }
                if (i + 1 != entries.Size()) entries[i] = Move(entries.Back());
                entries.PopBack();
                --m_Size;
            }
            if (entries.Empty()) empty.PushBack(hash);
        }
        for (usize hash : empty) m_Entries.Erase(hash);
    }

    void PipelineCache::Clear()
    {
        std::lock_guard lock(m_Mutex);
        m_Entries.Clear();
        m_Size = 0;
    }

    usize PipelineCache::Size() const
    {
        std::lock_guard lock(m_Mutex);
        return m_Size;
    }
} // namespace Vortex::RHI
