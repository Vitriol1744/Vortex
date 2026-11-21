/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Assertions.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Profiler.hpp>
#include <Vortex/Renderer/ShaderLibrary.hpp>

#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <thread>

import Prism.Containers.Span;
import Prism.Containers.Vector;
import Prism.Memory.Scope;
import Vortex.Renderer;
import Vortex.RHI.Vulkan;

namespace Vortex
{
    /// Everything the game thread hands the render thread for one frame.
    struct FramePacket
    {
        RenderGraph                     Graph;
        Vector<Renderer::RenderCommand> Commands;
        u64                             Number = 0;
    };

    /// RenderContext's state belongs to the render thread loop.
    struct RenderThreadAccess
    {
        static RenderContext Make() { return RenderContext(); }
        static RHI::CommandList* List(RenderContext& context) { return context.m_Cmd; }
    };

    RHI::Device& RenderContext::Device() { return Renderer::Device(); }

    RHI::CommandList& RenderContext::Cmd()
    {
        VtCoreAssertMsg(Renderer::IsRenderThread(),
                        "Renderer: RenderContext used off the render thread");
        if (!m_Cmd) m_Cmd = &Renderer::Device().BeginCommandList();
        return *m_Cmd;
    }

    void RenderContext::Present(Span<const ::Ref<RHI::SwapChain>> swapChains)
    {
        Renderer::Device().Submit(Cmd(), swapChains);
        m_Cmd = nullptr;
    }

    namespace Renderer
    {
        namespace
        {
            Scope<RHI::Device>           s_Device;
            RHI::API                     s_API = RHI::API::eVulkan;
            Scope<Vortex::ShaderLibrary> s_ShaderLibrary;
            Scope<RHI::PipelineCache>    s_Pipelines;

            ::Ref<RHI::Texture>          s_WhiteTexture;
            ::Ref<RHI::Sampler>          s_LinearSampler;
            ::Ref<RHI::Sampler>          s_NearestSampler;
            ::Ref<RHI::Sampler>          s_LinearClampSampler;

            // Game thread
            Scope<FramePacket>           s_Packet;
            u64                          s_FrameNumber = 0;

            /// The render thread and its one-slot mailbox.
            struct RenderThread
            {
                std::thread             Thread;
                std::thread::id         Id;
                std::mutex              Mutex;
                std::condition_variable Changed;
                Scope<FramePacket>      Pending;
                bool                    Executing = false;
                bool                    Running   = false;
                RGTransientPool         Transients;
            };
            RenderThread s_Thread;

            void ExecutePacket(FramePacket& packet)
            {
                VtProfileFunction();
                auto& device = *s_Device;
                device.BeginFrame();
                s_Pipelines->Collect(device.FrameNumber(), kPipelineMaxAge);

                RenderContext context = RenderThreadAccess::Make();
                {
                    VtProfileScope("Render graph");
                    packet.Graph.Execute(context.Cmd(), s_Thread.Transients);
                }
                for (auto& command : packet.Commands)
                {
                    VtProfileScope("Render command");
                    command(context);
                }
                if (auto* list = RenderThreadAccess::List(context))
                    device.Submit(*list);

                device.EndFrame();
                s_Thread.Transients.EndFrame();
            }

            void RenderThreadMain()
            {
                VtProfilerSetThreadName("Render Thread");
                while (true)
                {
                    Scope<FramePacket> packet;
                    {
                        std::unique_lock lock(s_Thread.Mutex);
                        s_Thread.Changed.wait(lock, [] {
                            return s_Thread.Pending || !s_Thread.Running;
                        });
                        if (!s_Thread.Pending) break;
                        packet             = Move(s_Thread.Pending);
                        s_Thread.Executing = true;
                    }
                    s_Thread.Changed.notify_all();

                    ExecutePacket(*packet);
                    // Dropped here, so refs captured by the frame are released
                    // after their commands were recorded.
                    packet.Reset();
                    VtProfileFrameMarkNamed("Render");

                    {
                        std::lock_guard lock(s_Thread.Mutex);
                        s_Thread.Executing = false;
                    }
                    s_Thread.Changed.notify_all();
                }
            }

            void StartRenderThread()
            {
                s_Thread.Running = true;
                s_Thread.Thread  = std::thread(RenderThreadMain);
                s_Thread.Id      = s_Thread.Thread.get_id();
            }
            void StopRenderThread()
            {
                if (!s_Thread.Thread.joinable()) return;
                Flush();
                {
                    std::lock_guard lock(s_Thread.Mutex);
                    s_Thread.Running = false;
                }
                s_Thread.Changed.notify_all();
                s_Thread.Thread.join();
                s_Thread.Id = {};
                s_Thread.Transients.Clear();
            }

            void CreateDefaults()
            {
                RHI::TextureDesc white{};
                white.Format    = RHI::Format::eRGBA8Unorm;
                white.Extent    = {1, 1, 1};
                white.DebugName = "Renderer: white";
                u32 pixel       = 0xffffffff;
                s_WhiteTexture  = s_Device->CreateTexture(white, &pixel, sizeof(pixel));

                RHI::SamplerDesc linear{};
                linear.DebugName = "Renderer: linear";
                s_LinearSampler  = s_Device->CreateSampler(linear);

                RHI::SamplerDesc clamp{};
                clamp.AddressU = clamp.AddressV = clamp.AddressW
                    = RHI::AddressMode::eClampToEdge;
                clamp.MaxAnisotropy  = 1.0f;
                clamp.DebugName      = "Renderer: linear clamp";
                s_LinearClampSampler = s_Device->CreateSampler(clamp);

                RHI::SamplerDesc nearest{};
                nearest.Min = nearest.Mag = nearest.Mip = RHI::Filter::eNearest;
                nearest.AddressU = nearest.AddressV = nearest.AddressW
                    = RHI::AddressMode::eClampToEdge;
                nearest.MaxAnisotropy = 1.0f;
                nearest.DebugName     = "Renderer: nearest";
                s_NearestSampler      = s_Device->CreateSampler(nearest);
            }
        } // namespace

        bool Initialize(const Specification& specification)
        {
            VtProfileFunction();
            VtCoreAssertMsg(!s_Device, "Renderer: Already initialized");

            Vulkan::RegisterBackend();

            RHI::DeviceDesc desc{};
            desc.API                = specification.API;
            s_API                   = specification.API;
            desc.ApplicationName    = specification.ApplicationName;
            desc.ApplicationVersion = specification.ApplicationVersion;
            desc.EngineName         = String(g_EngineName);
            desc.EngineVersion      = g_Version;
            desc.FramesInFlight     = specification.FramesInFlight;
            desc.PipelineCachePath  = specification.PipelineCachePath;
            desc.PreferredAdapter   = specification.PreferredAdapter;
            // VT_VULKAN_VALIDATION=0/1 overrides the build's default (on in
            // debug builds): validation costs a lot of CPU per draw.
            if (const char* validation = std::getenv("VT_VULKAN_VALIDATION"))
                desc.Validation = validation[0] != '0';

            auto device = RHI::Device::Create(desc);
            if (!device)
            {
                VtCoreError("Renderer: {}", device.Error());
                return false;
            }
            s_Device        = Move(*device);
            s_Pipelines     = CreateScope<RHI::PipelineCache>(*s_Device);
            s_ShaderLibrary = CreateScope<Vortex::ShaderLibrary>();
            CreateDefaults();
            StartRenderThread();
            return true;
        }

        void Shutdown()
        {
            VtProfileFunction();
            if (!s_Device) return;
            VtCoreAssertMsg(!s_Packet, "Renderer: Shutdown inside a frame");
            StopRenderThread();
            s_Device->WaitIdle();
            s_Pipelines.Reset();
            s_ShaderLibrary.Reset();
            s_WhiteTexture       = nullptr;
            s_LinearSampler      = nullptr;
            s_NearestSampler     = nullptr;
            s_LinearClampSampler = nullptr;
            s_Device.Reset();
        }
        bool IsInitialized() { return s_Device != nullptr; }

        Vortex::ShaderLibrary& ShaderLibrary()
        {
            VtCoreAssertMsg(s_ShaderLibrary, "Renderer: Not initialized");
            return *s_ShaderLibrary;
        }
        RHI::PipelineCache& Pipelines()
        {
            VtCoreAssertMsg(s_Pipelines, "Renderer: Not initialized");
            return *s_Pipelines;
        }
        RHI::API ActiveAPI() { return s_API; }

        RHI::Device& Device()
        {
            VtCoreAssertMsg(s_Device, "Renderer: Not initialized");
            return *s_Device;
        }

        void BeginFrame()
        {
            VtProfileFunction();
            VtCoreAssertMsg(!s_Packet, "Renderer: BeginFrame twice");
            s_Packet         = CreateScope<FramePacket>();
            s_Packet->Number = ++s_FrameNumber;
        }

        RenderGraph& Graph()
        {
            VtCoreAssertMsg(s_Packet, "Renderer: Graph() outside BeginFrame/EndFrame");
            return s_Packet->Graph;
        }

        void Submit(RenderCommand command)
        {
            VtCoreAssertMsg(s_Packet, "Renderer: Submit outside BeginFrame/EndFrame");
            s_Packet->Commands.PushBack(Move(command));
        }

        void EndFrame()
        {
            VtProfileFunction();
            VtCoreAssertMsg(s_Packet, "Renderer: EndFrame without BeginFrame");
            {
                // One frame in the mailbox at most: wait for the render thread
                // to take the previous one.
                std::unique_lock lock(s_Thread.Mutex);
                s_Thread.Changed.wait(lock, [] { return !s_Thread.Pending; });
                s_Thread.Pending = Move(s_Packet);
            }
            s_Thread.Changed.notify_all();
        }
        bool InFrame() { return s_Packet != nullptr; }
        u64  FrameNumber() { return s_FrameNumber; }

        void Flush()
        {
            VtProfileFunction();
            if (IsRenderThread() || !s_Thread.Thread.joinable()) return;
            std::unique_lock lock(s_Thread.Mutex);
            s_Thread.Changed.wait(lock, [] {
                return !s_Thread.Pending && !s_Thread.Executing;
            });
        }

        void WaitIdle()
        {
            if (!s_Device) return;
            Flush();
            s_Device->WaitIdle();
        }

        void ExecuteImmediate(Delegate<void(RHI::CommandList&)> commands)
        {
            VtCoreAssertMsg(!IsRenderThread(),
                            "Renderer: ExecuteImmediate from the render thread");
            Flush();
            s_Device->ExecuteImmediate(Move(commands));
        }

        RHI::MemoryStats MemoryStats() { return Device().QueryMemoryStats(); }
        bool IsRenderThread() { return std::this_thread::get_id() == s_Thread.Id; }

        ::Ref<RHI::Texture> WhiteTexture() { return s_WhiteTexture; }
        ::Ref<RHI::Sampler> LinearSampler() { return s_LinearSampler; }
        ::Ref<RHI::Sampler> NearestSampler() { return s_NearestSampler; }
        ::Ref<RHI::Sampler> LinearClampSampler() { return s_LinearClampSampler; }
    } // namespace Renderer
} // namespace Vortex
