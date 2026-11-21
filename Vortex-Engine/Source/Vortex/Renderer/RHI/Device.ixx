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

export module Vortex.RHI.Device;

export import Vortex.RHI.CommandList;
export import Vortex.RHI.SwapChain;

import Prism.Containers.Span;
import Prism.Containers.Vector;
import Prism.Core.Version;
import Prism.Memory.Ref;
import Prism.Memory.Scope;
import Prism.String.String;
import Prism.String.StringView;
import Prism.Utility.Delegate;
import Prism.Utility.Expected;
import Prism.Utility.Path;

/**
 * The RHI device: creates every resource, owns frames in flight, submits
 * command lists and presents.
 *
 * Frame protocol (render thread):
 *
 *   device.BeginFrame();                      // waits for frame N-FIF to retire
 *   if (swapChain->AcquireNextImage()) {
 *       auto& cmd = device.BeginCommandList();
 *       ... record ...
 *       device.Submit(cmd, {&swapChain, 1});  // waits for the image, presents
 *   }
 *   device.EndFrame();
 *
 * Threading contract:
 *  - Resource creation and destruction (dropping Refs) are thread-safe.
 *  - BeginFrame/EndFrame, BeginCommandList, Submit and command recording must
 *    happen on one thread at a time: the render thread. Checked in debug.
 *  - UploadBuffer/UploadTexture are thread-safe; uploads are recorded into the
 *    current frame and complete before the frame's first submitted command
 *    list executes.
 */
export namespace Vortex::RHI
{
    enum class API : u8
    {
        eVulkan,
    };

    /// Every API the engine has a backend for, in the order a UI lists them.
    inline constexpr API kAPIs[] = {API::eVulkan};

    inline const char* APIName(API api)
    {
        switch (api)
        {
            case API::eVulkan: return "Vulkan";
        }
        return "Unknown";
    }
    /// The API named `name` (as APIName spells it); false if there is none.
    inline bool ParseAPI(StringView name, API& out)
    {
        for (API api : kAPIs)
        {
            if (name != StringView(APIName(api))) continue;
            out = api;
            return true;
        }
        return false;
    }

    enum class AdapterType : u8
    {
        eOther,
        eIntegrated,
        eDiscrete,
        eVirtual,
        eCPU,
    };

    inline const char* AdapterTypeName(AdapterType type)
    {
        switch (type)
        {
            case AdapterType::eIntegrated: return "Integrated GPU";
            case AdapterType::eDiscrete: return "Discrete GPU";
            case AdapterType::eVirtual: return "Virtual GPU";
            case AdapterType::eCPU: return "CPU (software)";
            default: break;
        }
        return "Other";
    }

    /// A GPU the backend found, usable or not.
    struct AdapterInfo
    {
        String      Name{};
        AdapterType Type     = AdapterType::eOther;
        u32         VendorID = 0;
        u32         DeviceID = 0;
        /// "NVIDIA", "AMD", "Intel"... from the vendor id.
        String      Vendor{};
        /// Driver name and version, as the driver reports them.
        String      Driver{};
        /// Highest graphics API version the adapter supports ("1.3.280").
        String      APIVersion{};
        /// Device-local (VRAM) and host-visible-only heap sizes, in bytes.
        u64         DedicatedMemory = 0;
        u64         SharedMemory    = 0;
        /// Meets the engine's requirements; if not, why.
        bool        Suitable        = false;
        String      UnsuitableReason{};
    };

    struct DeviceDesc
    {
        RHI::API API                = RHI::API::eVulkan;
        String   ApplicationName{};
        Version  ApplicationVersion = {0, 1, 0};
        String   EngineName         = "Vortex Engine";
        Version  EngineVersion      = {0, 1, 0};
        u32      FramesInFlight     = 2;
        /// Where the backend keeps its compiled-pipeline cache between runs
        /// (loaded at creation, written at destruction). Empty: in memory
        /// only.
        Path     PipelineCachePath;
        /// Name of the adapter to use (AdapterInfo::Name). Empty, or not
        /// found / unsuitable: the backend picks the best one.
        String   PreferredAdapter{};
        /// Backend validation layers; on by default in debug builds.
#ifdef VT_DEBUG
        bool     Validation         = true;
#else
        bool     Validation         = false;
#endif
    };

    class Device;
    using DeviceFactory
        = Expected<Scope<Device>, String> (*)(const DeviceDesc& desc);

    class VT_RENDERER_RHI_API Device
    {
      public:
        Device(const Device&)            = delete;
        Device& operator=(const Device&) = delete;
        virtual ~Device();

        /// Backends register a factory for their API; the RHI itself doesn't
        /// link against any backend.
        static void RegisterAPI(RHI::API api, DeviceFactory factory);
        /// Creates the device for `desc.API` and makes it the global one.
        /// Fails if no backend registered that API or the backend can't
        /// initialize (no suitable GPU, missing driver features, ...).
        static Expected<Scope<Device>, String> Create(const DeviceDesc& desc);
        /// The device created by Create(). Asserts if there is none.
        static Device& Get();
        static bool    Exists();

        inline const DeviceDesc& Desc() const { return m_Desc; }
        inline RHI::API          API() const { return m_Desc.API; }

        //--------------------------------------------------------------------
        // Resources ---------------------------------------------------------
        //--------------------------------------------------------------------

        virtual ::Ref<Buffer>  CreateBuffer(const BufferDesc& desc) = 0;
        /// Creates and uploads `data` (`desc.Size` bytes). GPU-only buffers
        /// get TransferDst usage added automatically.
        ::Ref<Buffer>          CreateBuffer(const BufferDesc& desc,
                                            const void*       data);
        virtual ::Ref<Texture> CreateTexture(const TextureDesc& desc) = 0;
        /// Creates, uploads mip 0 from `data` and generates the remaining mips
        /// (if desc.MipLevels > 1). Ends in eShaderRead state.
        ::Ref<Texture>         CreateTexture(const TextureDesc& desc,
                                             const void* data, usize size);
        virtual ::Ref<Sampler> CreateSampler(const SamplerDesc& desc) = 0;
        virtual Expected<::Ref<Shader>, String>
        CreateShader(const ShaderDesc& desc)
            = 0;
        virtual ::Ref<Pipeline>  CreatePipeline(const GraphicsPipelineDesc& desc)
            = 0;
        virtual ::Ref<Pipeline>
        CreateComputePipeline(const ComputePipelineDesc& desc)
            = 0;
        virtual ::Ref<BindGroup> CreateBindGroup(const BindGroupDesc& desc) = 0;
        virtual ::Ref<SwapChain> CreateSwapChain(const SwapChainDesc& desc) = 0;

        //--------------------------------------------------------------------
        // Uploads -----------------------------------------------------------
        //--------------------------------------------------------------------

        /// Copies into any buffer (GPU-only included) through staging memory.
        /// The GPU sees the data from the current frame's first submit on.
        virtual void UploadBuffer(Buffer& buffer, const void* data, usize size,
                                  usize offset = 0)
            = 0;
        /// Copies into one region of a texture through staging memory. The
        /// texture ends in `finalState`.
        virtual void UploadTexture(Texture& texture, const void* data,
                                   usize size, const TextureRegion& region = {},
                                   ResourceState finalState
                                   = ResourceState::eShaderRead)
            = 0;
        /// Generates mips 1..N from mip 0 on the upload stream, after any
        /// upload made before it. Mip 0 must be in eTransferDst state (the
        /// state UploadTexture leaves it in when asked to).
        virtual void GenerateMips(Texture&      texture,
                                  ResourceState finalState
                                  = ResourceState::eShaderRead)
            = 0;

        //--------------------------------------------------------------------
        // Frames ------------------------------------------------------------
        //--------------------------------------------------------------------

        /// Waits until the frame that last used this frame slot has finished
        /// on the GPU, then frees resources it was the last user of.
        virtual void         BeginFrame() = 0;
        virtual void         EndFrame()   = 0;
        /// A command list recording into the current frame.
        virtual CommandList& BeginCommandList(QueueType queue
                                              = QueueType::eGraphics)
            = 0;
        /// Ends and submits `commands`. It waits for the swapchains' images to
        /// be acquired and presents them once it has executed.
        virtual void Submit(CommandList& commands,
                            Span<const ::Ref<SwapChain>> present = {})
            = 0;
        virtual void WaitIdle() = 0;

        /// Records `commands` and runs them to completion before returning,
        /// after every upload made so far. For load-time GPU work (baking
        /// environment maps, generating lookup tables); outside of frames.
        virtual void ExecuteImmediate(Delegate<void(CommandList&)> commands)
            = 0;

        /// Slot of the current frame, in [0, FramesInFlight).
        inline u32   FrameIndex() const { return m_FrameIndex; }
        /// Frames begun since the device was created.
        inline u64   FrameNumber() const { return m_FrameNumber; }
        inline u32   FramesInFlight() const { return m_Desc.FramesInFlight; }

        //--------------------------------------------------------------------
        // Queries -----------------------------------------------------------
        //--------------------------------------------------------------------

        virtual const DeviceLimits&   Limits() const   = 0;
        virtual const DeviceFeatures& Features() const = 0;
        virtual MemoryStats           QueryMemoryStats() const = 0;
        virtual String                AdapterName() const = 0;
        /// Every adapter the backend found when the device was created.
        virtual const Vector<AdapterInfo>& Adapters() const = 0;
        /// Index into Adapters() of the one this device runs on.
        virtual u32                   ActiveAdapter() const = 0;

      protected:
        explicit Device(const DeviceDesc& desc);

        DeviceDesc m_Desc;
        u32        m_FrameIndex  = 0;
        u64        m_FrameNumber = 0;
    };

    /// Shorthand for RHI::Device::Get().
    inline Device& GetDevice() { return Device::Get(); }
} // namespace Vortex::RHI
