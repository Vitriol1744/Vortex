/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/Export.hpp>

export module Vortex.Renderer;

export import Vortex.RHI;
export import Vortex.Renderer.RenderGraph;

import Prism.Containers.Span;
import Prism.Core.Version;
import Prism.Memory.Ref;
import Prism.String.String;
import Prism.Utility.Delegate;
import Prism.Utility.Path;

/**
 * The engine's rendering frontend: owns the RHI device and the render thread.
 *
 * The game (main) thread describes frame N+1 while the render thread records
 * and submits frame N:
 *
 *   Renderer::BeginFrame();
 *   SceneRenderer::AddToGraph(Renderer::Graph(), ...);   // passes
 *   Renderer::Submit([=](RenderContext& ctx) { ... });   // render-thread work
 *   Renderer::EndFrame();                                // hand over
 *
 * The game thread never records commands or writes per-frame GPU memory: it
 * captures values (draw lists, matrices, ImGui geometry) into passes and
 * commands; everything touching command lists or frame resources runs on the
 * render thread. Creating and destroying resources is fine on any thread.
 *
 * Exports are `extern "C++"` so engine headers can keep forward-declaring
 * the classes they use without attaching them to a module.
 */
export extern "C++"
{
    namespace Vortex
    {
        class ShaderLibrary;

        /// What render-thread commands get: the frame's main command list.
        class VT_RENDERER_API RenderContext
        {
          public:
            RHI::CommandList& Cmd();
            /// Submits everything recorded on Cmd() so far, presenting
            /// `swapChains`; Cmd() then starts a new list.
            void              Present(Span<const ::Ref<RHI::SwapChain>> swapChains);
            RHI::Device&      Device();

          private:
            RenderContext() = default;
            RHI::CommandList* m_Cmd = nullptr;
            friend struct RenderThreadAccess;
        };
    } // namespace Vortex

    namespace Vortex::Renderer
    {
        struct Specification
        {
            String  ApplicationName{};
            Version ApplicationVersion = {0, 1, 0};
            u32     FramesInFlight     = 2;
            /// Compiled pipelines are kept here between runs; empty keeps
            /// them in memory only.
            Path    PipelineCachePath;
            /// GPU to run on, by name (RHI::AdapterInfo::Name); empty or
            /// unavailable picks the best one.
            String  PreferredAdapter{};
            /// Graphics API to render with.
            RHI::API API = RHI::API::eVulkan;
        };

        using RenderCommand = Delegate<void(RenderContext&)>;

        VT_RENDERER_API bool Initialize(const Specification& specification);
        /// Drains and stops the render thread, then destroys the device.
        /// Every RHI resource must have been released before.
        VT_RENDERER_API void Shutdown();
        VT_RENDERER_API bool IsInitialized();

        VT_RENDERER_API RHI::Device&           Device();
        /// The graphics API the device runs on.
        VT_RENDERER_API RHI::API               ActiveAPI();
        /// Shaders shared by the renderers ("PBR", "Sprite", ...).
        VT_RENDERER_API Vortex::ShaderLibrary& ShaderLibrary();
        /// Pipelines made on demand from descriptions, so renderers can draw
        /// with whatever shader a material (or a hot reload) hands them.
        /// Thread-safe. Pipelines unused for kPipelineMaxAge frames are
        /// dropped.
        VT_RENDERER_API RHI::PipelineCache&    Pipelines();
        inline constexpr u64                   kPipelineMaxAge = 600;

        //--------------------------------------------------------------------
        // Game thread -------------------------------------------------------
        //--------------------------------------------------------------------

        /// Starts describing a frame.
        VT_RENDERER_API void         BeginFrame();
        /// The frame's render graph (executed first on the render thread).
        VT_RENDERER_API RenderGraph& Graph();
        /// Queues work for the render thread, run after the graph, in order.
        VT_RENDERER_API void         Submit(RenderCommand command);
        /// Hands the frame to the render thread. Blocks while the render
        /// thread is still a whole frame behind.
        VT_RENDERER_API void         EndFrame();
        VT_RENDERER_API bool         InFrame();
        /// Frames begun on the game thread.
        VT_RENDERER_API u64          FrameNumber();

        /// Waits until the render thread executed every frame handed to it.
        VT_RENDERER_API void Flush();
        /// Flush, then wait for the GPU.
        VT_RENDERER_API void WaitIdle();
        /// Flushes, then runs GPU work to completion (load-time baking).
        VT_RENDERER_API void ExecuteImmediate(Delegate<void(RHI::CommandList&)> commands);

        VT_RENDERER_API RHI::MemoryStats MemoryStats();
        /// Whether the calling thread is the render thread.
        VT_RENDERER_API bool             IsRenderThread();

        //--------------------------------------------------------------------
        // Defaults ----------------------------------------------------------
        //--------------------------------------------------------------------

        /// 1x1 opaque white, for unbound texture slots.
        VT_RENDERER_API ::Ref<RHI::Texture> WhiteTexture();
        /// Trilinear, anisotropic, repeating.
        VT_RENDERER_API ::Ref<RHI::Sampler> LinearSampler();
        /// Trilinear, clamped to the edges (lookup tables, render targets,
        /// cube maps).
        VT_RENDERER_API ::Ref<RHI::Sampler> LinearClampSampler();
        /// Point sampling, clamped to the edges (UI, pixel art).
        VT_RENDERER_API ::Ref<RHI::Sampler> NearestSampler();

        /// Format of the editor/game viewport targets the scene renders into.
        inline constexpr RHI::Format kSceneColorFormat = RHI::Format::eBGRA8Srgb;
        inline constexpr RHI::Format kSceneDepthFormat = RHI::Format::eD32Float;
    } // namespace Vortex::Renderer
}
