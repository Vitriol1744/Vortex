/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

#include <mutex>

export module Vortex.RHI.PipelineCache;

export import Vortex.RHI.Pipeline;
import Vortex.RHI.Device;

import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Prism.Memory.Ref;

/**
 * Pipelines on demand: describe the pipeline a draw needs, get the one made
 * for that description, created the first time it is asked for.
 *
 *   RHI::GraphicsPipelineDesc desc = ...;   // shader + fixed-function state
 *   desc.Shader = material->Shader();       // any shader, swappable any time
 *   cmd.BindPipeline(*cache.Get(desc));
 *
 * Renderers keep descriptions instead of pipelines, so changing a shader
 * (a material's, or a hot-reloaded one) or an attachment format simply asks
 * for a different pipeline. Descriptions match by value, except the shader,
 * which matches by identity, and DebugName, which is ignored.
 *
 * Entries nobody asked for during `maxAge` frames are dropped by Collect();
 * Invalidate() drops a replaced shader's pipelines right away. Dropping only
 * releases the cache's reference: draws already recorded keep theirs.
 *
 * Thread-safe. Creation happens on the calling thread, under no lock held by
 * other lookups of already-cached pipelines.
 */
export namespace Vortex::RHI
{
    /// Hash of everything that makes two descriptions different pipelines.
    VT_RENDERER_RHI_API usize HashOf(const GraphicsPipelineDesc& desc);
    VT_RENDERER_RHI_API usize HashOf(const ComputePipelineDesc& desc);
    /// Whether both describe the same pipeline (DebugName aside).
    VT_RENDERER_RHI_API bool  Equivalent(const GraphicsPipelineDesc& lhs,
                                         const GraphicsPipelineDesc& rhs);

    class VT_RENDERER_RHI_API PipelineCache
    {
      public:
        /// Creates pipelines on `device`.
        explicit PipelineCache(Device& device);
        PipelineCache(const PipelineCache&)            = delete;
        PipelineCache& operator=(const PipelineCache&) = delete;
        ~PipelineCache();

        /// The pipeline for `desc`; null if `desc` has no shader.
        ::Ref<Pipeline> Get(const GraphicsPipelineDesc& desc);
        ::Ref<Pipeline> Get(const ComputePipelineDesc& desc);

        /// Drops every pipeline made from `shader` (it was replaced).
        void            Invalidate(const Shader& shader);
        /// Advances the cache's clock to `frame` and drops the pipelines not
        /// asked for since frame `frame - maxAge`.
        void            Collect(u64 frame, u64 maxAge);
        void            Clear();

        usize           Size() const;
        /// Pipelines created by this cache so far (misses).
        inline u64      CreatedCount() const { return m_Created; }

      private:
        struct Entry
        {
            ::Ref<Pipeline> Pipeline;
            u64             LastUsed = 0;
        };

        Device&                             m_Device;
        mutable std::mutex                  m_Mutex;
        /// Hash -> pipelines with that hash (collisions are compared).
        UnorderedMap<usize, Vector<Entry>>  m_Entries;
        usize                               m_Size    = 0;
        u64                                 m_Frame   = 0;
        u64                                 m_Created = 0;

        ::Ref<Pipeline> Find(usize hash, PipelineType type, const void* desc);
        ::Ref<Pipeline> Insert(usize hash, PipelineType type, const void* desc,
                               ::Ref<Pipeline> created);
    };
} // namespace Vortex::RHI
