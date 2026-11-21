/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanInstance.hpp>

#include <mutex>
#include <thread>

import Prism.Containers.Span;
import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Memory.Scope;
import Prism.Utility.Delegate;
import Prism.Utility.Expected;
import Prism.Utility.Optional;

namespace Vortex::Vulkan
{
    class VulkanCommandList;
    class VulkanSwapChain;
    class VulkanTexture;

    struct QueueFamilies
    {
        /// Graphics + compute + transfer, and able to present.
        Optional<u32> Graphics;
    };

    /// Native objects whose destruction waits for the GPU (see Defer()).
    struct DeferredDeletion
    {
        enum class Type : u8
        {
            eBuffer,
            eImage,
            eImageView,
            eSampler,
            ePipeline,
            ePipelineLayout,
            eDescriptorSetLayout,
            eDescriptorSet,
            eShaderModule,
            eSwapChain,
            eSurface,
            eSemaphore,
        };

        Type          Kind;
        u64           Handle     = 0;
        /// Descriptor pool of a descriptor set.
        u64           Owner      = 0;
        VmaAllocation Allocation = VK_NULL_HANDLE;
        /// Frame number after which the GPU no longer uses the object.
        u64           Frame      = 0;
    };

    class VT_RENDERER_RHI_VULKAN_API VulkanDevice final : public RHI::Device
    {
      public:
        static Expected<Scope<RHI::Device>, String>
        Create(const RHI::DeviceDesc& desc);
        static inline VulkanDevice& Get()
        {
            return static_cast<VulkanDevice&>(RHI::Device::Get());
        }

        ~VulkanDevice() override;

        //--------------------------------------------------------------------
        // RHI::Device -------------------------------------------------------
        //--------------------------------------------------------------------

        using RHI::Device::CreateBuffer;
        using RHI::Device::CreateTexture;

        ::Ref<RHI::Buffer>  CreateBuffer(const RHI::BufferDesc& desc) override;
        ::Ref<RHI::Texture> CreateTexture(const RHI::TextureDesc& desc) override;
        ::Ref<RHI::Sampler> CreateSampler(const RHI::SamplerDesc& desc) override;
        Expected<::Ref<RHI::Shader>, String>
        CreateShader(const RHI::ShaderDesc& desc) override;
        ::Ref<RHI::Pipeline>
        CreatePipeline(const RHI::GraphicsPipelineDesc& desc) override;
        ::Ref<RHI::Pipeline>
        CreateComputePipeline(const RHI::ComputePipelineDesc& desc) override;
        ::Ref<RHI::BindGroup>
        CreateBindGroup(const RHI::BindGroupDesc& desc) override;
        ::Ref<RHI::SwapChain>
             CreateSwapChain(const RHI::SwapChainDesc& desc) override;

        void UploadBuffer(RHI::Buffer& buffer, const void* data, usize size,
                          usize offset) override;
        void UploadTexture(RHI::Texture& texture, const void* data, usize size,
                           const RHI::TextureRegion& region,
                           RHI::ResourceState        finalState) override;
        void GenerateMips(RHI::Texture&      texture,
                          RHI::ResourceState finalState) override;

        void BeginFrame() override;
        void EndFrame() override;
        RHI::CommandList& BeginCommandList(RHI::QueueType queue) override;
        void Submit(RHI::CommandList&                 commands,
                    Span<const ::Ref<RHI::SwapChain>> present) override;
        void WaitIdle() override;
        void ExecuteImmediate(Delegate<void(RHI::CommandList&)> commands) override;

        const RHI::DeviceLimits&   Limits() const override { return m_Limits; }
        const RHI::DeviceFeatures& Features() const override
        {
            return m_Features;
        }
        RHI::MemoryStats QueryMemoryStats() const override;
        String           AdapterName() const override;
        const Vector<RHI::AdapterInfo>& Adapters() const override
        {
            return m_Adapters;
        }
        u32 ActiveAdapter() const override { return m_ActiveAdapter; }

        //--------------------------------------------------------------------
        // Backend -----------------------------------------------------------
        //--------------------------------------------------------------------

        inline vk::Device         Handle() const { return m_Device; }
        inline vk::PhysicalDevice Physical() const { return m_Physical; }
        inline vk::Instance       Instance() const { return m_Instance; }
        inline VmaAllocator       Allocator() const { return m_Allocator; }
        inline vk::Queue          GraphicsQueue() const { return m_GraphicsQueue; }
        inline u32                GraphicsFamily() const
        {
            return m_Families.Graphics.Value();
        }
        inline bool DebugUtils() const { return m_Instance.DebugUtilsEnabled(); }

        /// Frees `handle` once every frame that may use it has completed.
        /// Thread-safe.
        void Defer(DeferredDeletion::Type type, u64 handle,
                   VmaAllocation allocation = VK_NULL_HANDLE, u64 owner = 0);
        template <typename Handle>
        inline void Defer(DeferredDeletion::Type type, Handle handle,
                          VmaAllocation allocation = VK_NULL_HANDLE)
        {
            using CType = typename Handle::CType;
            if (!handle) return;
            Defer(type, reinterpret_cast<u64>(static_cast<CType>(handle)),
                  allocation);
        }

        /// Allocates a set for `layout` from the shared descriptor pools.
        /// Thread-safe. Free it with Defer(eDescriptorSet, set, pool).
        vk::DescriptorSet AllocateDescriptorSet(vk::DescriptorSetLayout layout,
                                                vk::DescriptorPool& outPool);

        /// Empty set layout, for set indices a pipeline doesn't use.
        inline vk::DescriptorSetLayout EmptySetLayout() const
        {
            return m_EmptySetLayout;
        }

        /// The driver's pipeline cache every pipeline is created through:
        /// variants of a shader (other formats, blend or depth state) reuse
        /// its compiled stages. Persisted to DeviceDesc::PipelineCachePath.
        inline vk::PipelineCache PipelineCache() const
        {
            return m_PipelineCache;
        }

        /// Asserts that frame/recording calls come from one thread.
        void CheckRenderThread();

        /// Tracy's GPU timing context (tracy::VkCtx*), null without
        /// profiling.
        inline void* TracyContext() const { return m_TracyContext; }

      private:
        explicit VulkanDevice(const RHI::DeviceDesc& desc);

        Expected<void, String> Initialize();
        Expected<void, String> PickPhysicalDevice();
        Expected<void, String> CreateLogicalDevice();
        void                   CreateAllocator();
        void                   CreateFrames();
        void                   QueryLimits();
        void                   CreatePipelineCache();
        void                   SavePipelineCache();

        //--------------------------------------------------------------------
        // Uploads -----------------------------------------------------------
        //--------------------------------------------------------------------

        struct StagingChunk
        {
            vk::Buffer    Buffer     = VK_NULL_HANDLE;
            VmaAllocation Allocation = VK_NULL_HANDLE;
            u8*           Mapped     = nullptr;
            usize         Size       = 0;
        };
        /// Linear staging memory, reset whenever its commands have completed.
        struct StagingRing
        {
            Vector<StagingChunk> Chunks;
            usize                Chunk  = 0;
            usize                Offset = 0;
        };
        /// A command buffer recording uploads, and the staging it reads.
        struct UploadContext
        {
            vk::CommandPool   Pool    = VK_NULL_HANDLE;
            vk::CommandBuffer Buffer  = VK_NULL_HANDLE;
            bool              Recording = false;
            StagingRing       Staging;
        };
        struct StagingAllocation
        {
            vk::Buffer Buffer = VK_NULL_HANDLE;
            usize      Offset = 0;
            u8*        Mapped = nullptr;
        };

        /// The upload context to record into; locks m_UploadMutex first.
        UploadContext&    CurrentUploads();
        vk::CommandBuffer BeginUploads(UploadContext& context);
        StagingAllocation AllocateStaging(StagingRing& ring, usize size,
                                          usize alignment);
        void              ResetStaging(StagingRing& ring);
        void              DestroyStaging(StagingRing& ring);
        /// Ends the context's recording and returns its command buffer, or
        /// null if nothing was recorded.
        vk::CommandBuffer EndUploads(UploadContext& context);
        /// Submits uploads recorded outside of a frame.
        void              FlushIdleUploads();

        //--------------------------------------------------------------------
        // Frames ------------------------------------------------------------
        //--------------------------------------------------------------------

        struct Frame
        {
            vk::CommandPool                  Pool  = VK_NULL_HANDLE;
            Vector<Scope<VulkanCommandList>> Lists;
            u32                              ListsUsed = 0;
            vk::Fence                        Fence = VK_NULL_HANDLE;
            UploadContext                    Uploads;
        };

        void CollectGarbage(u64 completedFrame, bool all = false);
        void DestroyNow(const DeferredDeletion& entry);

        VulkanInstance       m_Instance;
        vk::PhysicalDevice   m_Physical = VK_NULL_HANDLE;
        vk::PhysicalDeviceProperties m_Properties{};
        QueueFamilies        m_Families;
        vk::Device           m_Device        = VK_NULL_HANDLE;
        vk::Queue            m_GraphicsQueue = VK_NULL_HANDLE;
        VmaAllocator         m_Allocator     = VK_NULL_HANDLE;
        bool                 m_MemoryBudget  = false;

        RHI::DeviceLimits    m_Limits;
        RHI::DeviceFeatures  m_Features;
        Vector<RHI::AdapterInfo> m_Adapters;
        u32                  m_ActiveAdapter = 0;

        Vector<Frame>        m_Frames;
        bool                 m_InFrame = false;

        std::mutex           m_UploadMutex;
        /// Uploads made between frames (e.g. while loading assets); flushed
        /// at the next BeginFrame.
        UploadContext        m_IdleUploads;
        vk::Fence            m_IdleUploadFence = VK_NULL_HANDLE;
        bool                 m_IdleUploadsInFlight = false;

        std::mutex               m_DeletionMutex;
        Vector<DeferredDeletion> m_Deletions;

        std::mutex                 m_DescriptorMutex;
        Vector<vk::DescriptorPool> m_DescriptorPools;
        vk::DescriptorSetLayout    m_EmptySetLayout = VK_NULL_HANDLE;
        vk::PipelineCache          m_PipelineCache  = VK_NULL_HANDLE;

        std::thread::id            m_RenderThread{};
        void*                      m_TracyContext = nullptr;
        /// Timestamps are collected once per frame, by its first command list.
        bool                       m_TracyCollected = false;
        void                       CreateTracyContext();

        friend class VulkanCommandList;
        friend class VulkanSwapChain;
    };
} // namespace Vortex::Vulkan
