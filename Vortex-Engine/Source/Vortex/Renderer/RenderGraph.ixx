/*
 * Created by v1tr10l7 on 17.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

export module Vortex.Renderer.RenderGraph;

export import Vortex.RHI;

import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Memory.Scope;
import Prism.String.String;
import Prism.String.StringView;

/**
 * A frame described as passes and the resources they read and write:
 *
 *   struct SceneData { RGTexture Color, Depth; };
 *   graph.AddPass<SceneData>("Scene",
 *       [&](RGBuilder& builder, SceneData& data) {
 *           data.Color = builder.WriteColor(graph.Import(target->Color()));
 *           data.Depth = builder.WriteDepth(
 *               builder.CreateTexture({.Format = ..., .Extent = ...}));
 *       },
 *       [=](const SceneData& data, RGContext& ctx) {
 *           ctx.Cmd().BindPipeline(...); ...  // rendering already begun
 *       });
 *
 * The setup callback runs immediately (on the thread building the frame),
 * the execute callback later, on the render thread, so it must capture by
 * value. Executing the graph:
 *  - culls passes that contribute nothing to an imported resource (or are
 *    marked SideEffect),
 *  - creates transient textures (pooled across frames),
 *  - inserts the barriers between declared accesses,
 *  - begins/ends rendering for passes that declared attachments,
 *  - wraps every pass in a debug label and a GPU timing zone,
 *  - leaves imported textures in their declared final state.
 */
export extern "C++"
{
    namespace Vortex
    {
        struct RGTexture
        {
            u32            Index = ~0u;
            constexpr bool Valid() const { return Index != ~0u; }
        };
        struct RGBuffer
        {
            u32            Index = ~0u;
            constexpr bool Valid() const { return Index != ~0u; }
        };

        class RenderGraph;
        class RGTransientPool;

        /// What a pass's execute callback gets.
        class VT_RENDERER_API RGContext
        {
          public:
            inline RHI::CommandList& Cmd() const { return m_Cmd; }
            ::Ref<RHI::Texture>      Get(RGTexture texture) const;
            ::Ref<RHI::Buffer>       Get(RGBuffer buffer) const;

          private:
            RGContext(RenderGraph& graph, RHI::CommandList& cmd)
                : m_Graph(graph)
                , m_Cmd(cmd)
            {
            }
            RenderGraph&      m_Graph;
            RHI::CommandList& m_Cmd;
            friend class RenderGraph;
        };

        namespace Details
        {
            struct RGAccess
            {
                u32                Resource = 0;
                bool               Texture  = true;
                RHI::ResourceState State    = RHI::ResourceState::eUndefined;
                bool               Write    = false;
                bool               Read     = false;
            };
            struct RGAttachment
            {
                u32                    Resource = 0;
                bool                   Depth    = false;
                bool                   ReadOnly = false;
                RHI::LoadOp            Load     = RHI::LoadOp::eClear;
                RHI::ClearColor        ClearColor{};
                RHI::ClearDepthStencil ClearDepth{};
            };

            class VT_RENDERER_API RGPass
            {
              public:
                virtual ~RGPass() = default;
                virtual void Execute(RGContext& context) = 0;

                String               Name{};
                Vector<RGAccess>     Accesses;
                Vector<RGAttachment> Attachments;
                bool                 SideEffect = false;
                bool                 Culled     = false;
            };

            template <typename Data, typename Function>
            class RGTypedPass final : public RGPass
            {
              public:
                explicit RGTypedPass(Function&& function)
                    : m_Function(static_cast<Function&&>(function))
                {
                }
                void Execute(RGContext& context) override
                {
                    m_Function(static_cast<const Data&>(PassData), context);
                }

                Data     PassData{};

              private:
                Function m_Function;
            };
        } // namespace Details

        /// Declares a pass's resources during setup.
        class VT_RENDERER_API RGBuilder
        {
          public:
            /// A texture that lives for this graph only (pooled).
            RGTexture CreateTexture(const RHI::TextureDesc& desc);

            /// Sampled / read by a shader (or another read state).
            RGTexture Read(RGTexture texture, RHI::ResourceState state
                                              = RHI::ResourceState::eShaderRead);
            /// Written by a shader as storage (or another write state).
            RGTexture Write(RGTexture texture, RHI::ResourceState state
                                               = RHI::ResourceState::eShaderWrite);
            /// Rendered into as a color attachment.
            RGTexture WriteColor(RGTexture texture, RHI::LoadOp load = RHI::LoadOp::eClear,
                                 RHI::ClearColor clear = {});
            /// The depth attachment, written.
            RGTexture WriteDepth(RGTexture texture, RHI::LoadOp load = RHI::LoadOp::eClear,
                                 RHI::ClearDepthStencil clear = {});
            /// The depth attachment, tested only.
            RGTexture ReadDepth(RGTexture texture);

            RGBuffer  Read(RGBuffer buffer, RHI::ResourceState state);
            RGBuffer  Write(RGBuffer buffer, RHI::ResourceState state);

            /// Never culled (e.g. it presents, reads back or has CPU effects).
            void      SideEffect();

          private:
            RGBuilder(RenderGraph& graph, Details::RGPass& pass)
                : m_Graph(graph)
                , m_Pass(pass)
            {
            }
            void             Access(u32 resource, bool texture,
                                    RHI::ResourceState state, bool read, bool write);
            RenderGraph&     m_Graph;
            Details::RGPass& m_Pass;
            friend class RenderGraph;
        };

        class VT_RENDERER_API RenderGraph
        {
          public:
            RenderGraph() = default;
            RenderGraph(const RenderGraph&)            = delete;
            RenderGraph& operator=(const RenderGraph&) = delete;

            /// An existing texture. Writing it makes a pass worth keeping;
            /// the graph leaves it in `finalState`.
            RGTexture Import(::Ref<RHI::Texture> texture,
                             RHI::ResourceState  finalState = RHI::ResourceState::eShaderRead);
            RGBuffer  Import(::Ref<RHI::Buffer> buffer);

            template <typename Data, typename Setup, typename Execute>
            const Data& AddPass(StringView name, Setup&& setup, Execute&& execute)
            {
                using Pass  = Details::RGTypedPass<Data, Execute>;
                auto  pass  = new Pass(static_cast<Execute&&>(execute));
                pass->Name  = String(name);
                m_Passes.PushBack(Scope<Details::RGPass>(pass));
                RGBuilder builder(*this, *pass);
                setup(builder, pass->PassData);
                return pass->PassData;
            }

            /// Culls, then records every live pass on `cmd` (render thread).
            void   Execute(RHI::CommandList& cmd, RGTransientPool& pool);

            inline usize PassCount() const { return m_Passes.Size(); }
            usize        CulledCount() const;
            /// The graph in Graphviz dot format (passes, resources, culling).
            String       Graphviz() const;

          private:
            struct Resource
            {
                String              Name{};
                bool                Texture  = true;
                bool                Imported = false;
                RHI::TextureDesc    Desc{};
                ::Ref<RHI::Texture> TextureRef;
                ::Ref<RHI::Buffer>  BufferRef;
                RHI::ResourceState  FinalState = RHI::ResourceState::eShaderRead;
                /// Graph-local state of buffers (textures track their own).
                RHI::ResourceState  BufferState = RHI::ResourceState::eUndefined;
            };

            Vector<Scope<Details::RGPass>> m_Passes;
            Vector<Resource>               m_Resources;

            void Cull();

            friend class RGBuilder;
            friend class RGContext;
        };

        /**
         * Transient textures, reused across frames by description. Reuse is
         * safe: barriers order a new frame's writes after the previous
         * frame's reads on the queue.
         */
        class VT_RENDERER_API RGTransientPool
        {
          public:
            ::Ref<RHI::Texture> Acquire(const RHI::TextureDesc& desc);
            /// Makes this frame's textures available again and drops ones
            /// unused for a while.
            void                EndFrame();
            void                Clear() { m_Entries.Clear(); }

          private:
            struct Entry
            {
                RHI::TextureDesc    Desc{};
                ::Ref<RHI::Texture> Texture;
                bool                InUse  = false;
                u32                 Unused = 0;
            };
            Vector<Entry> m_Entries;
        };
    } // namespace Vortex
}
