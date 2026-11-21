/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Core/Platform.hpp>

// Android has no prebuilt VMA library: compile the implementation here, with
// function pointers fetched at runtime (the loader isn't linked statically).
#ifdef VT_PLATFORM_ANDROID
    #define VMA_IMPLEMENTATION
    #define VMA_STATIC_VULKAN_FUNCTIONS  0
    #define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif

// Tracy's Vulkan header has C headers: before VulkanCommon.hpp's imports.
#ifdef TRACY_ENABLE
    #include <vulkan/vulkan.h>
    #include <tracy/TracyVulkan.hpp>
#endif
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommandList.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanPipeline.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanResources.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanShader.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSwapChain.hpp>

#include <algorithm>
#include <string_view>
#include <cstring>
#include <fstream>
#include <vector>
#include <limits>

import Prism.String.Formatter;
import Vortex.RHI.Vulkan;

namespace Vortex::Vulkan
{
    using namespace RHI;
    using Deletion = DeferredDeletion::Type;

    namespace
    {
        constexpr usize kStagingChunkSize = 16 * 1024 * 1024;
        constexpr u32   kDescriptorPoolSets = 512;

        bool HasDeviceExtension(vk::PhysicalDevice device, const char* name)
        {
            u32 count = 0;
            VkCall(device.enumerateDeviceExtensionProperties(nullptr, &count,
                                                             nullptr));
            Vector<vk::ExtensionProperties> extensions(count);
            VkCall(device.enumerateDeviceExtensionProperties(nullptr, &count,
                                                             extensions.Raw()));
            for (const auto& extension : extensions)
                if (std::strcmp(extension.extensionName, name) == 0) return true;
            return false;
        }

        u32 DestroyOrder(DeferredDeletion::Type type)
        {
            switch (type)
            {
                case DeferredDeletion::Type::eSwapChain: return 0;
                case DeferredDeletion::Type::eSemaphore: return 1;
                case DeferredDeletion::Type::eSurface: return 2;
                case DeferredDeletion::Type::eDescriptorSet: return 3;
                case DeferredDeletion::Type::eImageView: return 4;
                case DeferredDeletion::Type::eSampler: return 5;
                case DeferredDeletion::Type::ePipeline: return 6;
                case DeferredDeletion::Type::ePipelineLayout: return 7;
                case DeferredDeletion::Type::eDescriptorSetLayout: return 8;
                case DeferredDeletion::Type::eShaderModule: return 9;
                case DeferredDeletion::Type::eImage: return 10;
                case DeferredDeletion::Type::eBuffer: return 11;
            }
            return 12;
        }

        usize AlignUp(usize value, usize alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }
    } // namespace

    //------------------------------------------------------------------------
    // Creation --------------------------------------------------------------
    //------------------------------------------------------------------------

    Expected<Scope<RHI::Device>, String>
    VulkanDevice::Create(const DeviceDesc& desc)
    {
        // Constructed through `new`: the constructor is private and
        // Scope's factory can't reach it.
        Scope<VulkanDevice> device(new VulkanDevice(desc));
        auto                result = device->Initialize();
        if (!result) return Unexpected(result.Error());
        return Scope<RHI::Device>(device.Release());
    }

    VulkanDevice::VulkanDevice(const DeviceDesc& desc)
        : RHI::Device(desc)
    {
    }

    Expected<void, String> VulkanDevice::Initialize()
    {
        auto instance = m_Instance.Initialize(
            m_Desc.ApplicationName, m_Desc.ApplicationVersion,
            m_Desc.EngineName, m_Desc.EngineVersion, m_Desc.Validation);
        if (!instance) return instance;

        if (auto result = PickPhysicalDevice(); !result) return result;
        if (auto result = CreateLogicalDevice(); !result) return result;
        CreateAllocator();
        QueryLimits();
        CreateFrames();
        CreatePipelineCache();

        vk::DescriptorSetLayoutCreateInfo emptyInfo{};
        VkCall(m_Device.createDescriptorSetLayout(&emptyInfo, nullptr,
                                                  &m_EmptySetLayout));

        vk::FenceCreateInfo fenceInfo{};
        VkCall(m_Device.createFence(&fenceInfo, nullptr, &m_IdleUploadFence));
        CreateTracyContext();

        VtCoreInfoTag("Renderer", "Vulkan: Using '{}' ({} frames in flight)",
                      AdapterName(), m_Desc.FramesInFlight);
        return {};
    }

    void VulkanDevice::CreatePipelineCache()
    {
        // Seed the cache from disk when the data was made by this very
        // driver and GPU; anything else is ignored (drivers may reject or,
        // worse, misbehave on foreign data).
        std::vector<char> data;
        if (!m_Desc.PipelineCachePath.Empty())
        {
            std::ifstream file(m_Desc.PipelineCachePath.Raw(),
                               std::ios::binary | std::ios::ate);
            if (file.is_open())
            {
                data.resize(static_cast<usize>(file.tellg()));
                file.seekg(0);
                file.read(data.data(), static_cast<std::streamsize>(data.size()));
                if (!file) data.clear();
            }

            VkPipelineCacheHeaderVersionOne header{};
            bool valid = data.size() >= sizeof(header);
            if (valid)
            {
                std::memcpy(&header, data.data(), sizeof(header));
                valid = header.headerSize >= sizeof(header)
                     && header.headerVersion
                            == VK_PIPELINE_CACHE_HEADER_VERSION_ONE
                     && header.vendorID == m_Properties.vendorID
                     && header.deviceID == m_Properties.deviceID
                     && std::memcmp(header.pipelineCacheUUID,
                                    m_Properties.pipelineCacheUUID,
                                    VK_UUID_SIZE)
                            == 0;
            }
            if (!valid && !data.empty())
                VtCoreInfoTag("Renderer",
                              "Vulkan: Pipeline cache '{}' is from another "
                              "driver or GPU; starting empty",
                              m_Desc.PipelineCachePath.Raw());
            if (!valid) data.clear();
        }

        vk::PipelineCacheCreateInfo info{};
        info.initialDataSize = data.size();
        info.pInitialData    = data.empty() ? nullptr : data.data();
        VkCall(m_Device.createPipelineCache(&info, nullptr, &m_PipelineCache));
    }

    void VulkanDevice::SavePipelineCache()
    {
        if (!m_PipelineCache || m_Desc.PipelineCachePath.Empty()) return;
        usize size = 0;
        if (m_Device.getPipelineCacheData(m_PipelineCache, &size, nullptr)
                != vk::Result::eSuccess
            || size == 0)
            return;
        std::vector<char> data(size);
        if (m_Device.getPipelineCacheData(m_PipelineCache, &size, data.data())
            != vk::Result::eSuccess)
            return;

        std::ofstream file(m_Desc.PipelineCachePath.Raw(),
                           std::ios::binary | std::ios::trunc);
        if (!file.is_open()
            || !file.write(data.data(), static_cast<std::streamsize>(size)))
            VtCoreWarn("Vulkan: Can't write the pipeline cache to '{}'",
                       m_Desc.PipelineCachePath.Raw());
    }

    void VulkanDevice::CreateTracyContext()
    {
#ifdef TRACY_ENABLE
        // The context calibrates with one command buffer of its own.
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = GraphicsFamily();
        vk::CommandPool pool      = VK_NULL_HANDLE;
        VkCall(m_Device.createCommandPool(&poolInfo, nullptr, &pool));
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.commandPool        = pool;
        allocInfo.level              = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;
        vk::CommandBuffer buffer     = VK_NULL_HANDLE;
        VkCall(m_Device.allocateCommandBuffers(&allocInfo, &buffer));

        m_TracyContext = tracy::CreateVkContext(m_Physical, m_Device, m_GraphicsQueue,
                                                buffer, nullptr, nullptr);
        m_Device.waitIdle();
        m_Device.destroyCommandPool(pool);
#endif
    }

    namespace
    {
        RHI::AdapterType AdapterTypeOf(vk::PhysicalDeviceType type)
        {
            switch (type)
            {
                case vk::PhysicalDeviceType::eIntegratedGpu:
                    return RHI::AdapterType::eIntegrated;
                case vk::PhysicalDeviceType::eDiscreteGpu:
                    return RHI::AdapterType::eDiscrete;
                case vk::PhysicalDeviceType::eVirtualGpu:
                    return RHI::AdapterType::eVirtual;
                case vk::PhysicalDeviceType::eCpu: return RHI::AdapterType::eCPU;
                default: break;
            }
            return RHI::AdapterType::eOther;
        }

        String VendorName(u32 vendor)
        {
            switch (vendor)
            {
                case 0x10de: return "NVIDIA";
                case 0x1002: return "AMD";
                case 0x8086: return "Intel";
                case 0x13b5: return "ARM";
                case 0x5143: return "Qualcomm";
                case 0x1010: return "Imagination";
                case 0x106b: return "Apple";
                case 0x10005: return "Mesa";
                default: break;
            }
            return Prism::Format("Unknown (0x{:04x})", vendor);
        }

        String VersionString(u32 version)
        {
            return Prism::Format("{}.{}.{}", VK_API_VERSION_MAJOR(version),
                                 VK_API_VERSION_MINOR(version),
                                 VK_API_VERSION_PATCH(version));
        }
    } // namespace

    Expected<void, String> VulkanDevice::PickPhysicalDevice()
    {
        u32 count = 0;
        VkCall(m_Instance.Handle().enumeratePhysicalDevices(&count, nullptr));
        Vector<vk::PhysicalDevice> devices(count);
        VkCall(m_Instance.Handle().enumeratePhysicalDevices(&count, devices.Raw()));
        if (devices.Empty())
            return Unexpected(String("Vulkan: No GPU with Vulkan support"));

        m_Adapters.Clear();
        u64  bestScore     = 0;
        bool havePreferred = false;
        for (auto candidate : devices)
        {
            vk::PhysicalDeviceProperties properties{};
            candidate.getProperties(&properties);
            String name(properties.deviceName.data());

            RHI::AdapterInfo& info = m_Adapters.EmplaceBack();
            info.Name              = name;
            info.Type              = AdapterTypeOf(properties.deviceType);
            info.VendorID          = properties.vendorID;
            info.DeviceID          = properties.deviceID;
            info.Vendor            = VendorName(properties.vendorID);
            info.APIVersion        = VersionString(properties.apiVersion);
            info.Driver            = Prism::Format("0x{:x}", properties.driverVersion);
            if (properties.apiVersion >= VK_API_VERSION_1_2)
            {
                vk::PhysicalDeviceDriverProperties driver{};
                vk::PhysicalDeviceProperties2      properties2{};
                properties2.pNext = &driver;
                candidate.getProperties2(&properties2);
                info.Driver = Prism::Format("{} {}", StringView(driver.driverName.data()),
                                            StringView(driver.driverInfo.data()));
            }

            vk::PhysicalDeviceMemoryProperties memory{};
            candidate.getMemoryProperties(&memory);
            for (u32 i = 0; i < memory.memoryHeapCount; ++i)
            {
                if (memory.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                    info.DedicatedMemory += memory.memoryHeaps[i].size;
                else info.SharedMemory += memory.memoryHeaps[i].size;
            }

            auto reject = [&](StringView reason)
            {
                info.UnsuitableReason = String(reason);
                VtCoreTrace("Vulkan: Skipping '{}': {}", name, reason);
            };

            if (properties.apiVersion < kRequiredApiVersion)
            {
                reject("no Vulkan 1.3");
                continue;
            }
            if (!HasDeviceExtension(candidate, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            {
                reject("no swapchain support");
                continue;
            }

            vk::PhysicalDeviceVulkan13Features features13{};
            vk::PhysicalDeviceFeatures2        features{};
            features.pNext = &features13;
            candidate.getFeatures2(&features);
            if (!features13.dynamicRendering || !features13.synchronization2)
            {
                reject("no dynamic rendering or synchronization2");
                continue;
            }

            // One queue family doing graphics, compute and transfer (graphics
            // implies transfer). Presentation support is checked per surface.
            u32 familyCount = 0;
            candidate.getQueueFamilyProperties(&familyCount, nullptr);
            Vector<vk::QueueFamilyProperties> families(familyCount);
            candidate.getQueueFamilyProperties(&familyCount, families.Raw());
            Optional<u32> graphics;
            for (u32 i = 0; i < familyCount && !graphics.HasValue(); ++i)
            {
                auto flags = families[i].queueFlags;
                if ((flags & vk::QueueFlagBits::eGraphics)
                    && (flags & vk::QueueFlagBits::eCompute))
                    graphics = i;
            }
            if (!graphics.HasValue())
            {
                reject("no graphics queue");
                continue;
            }
            info.Suitable = true;

            // The preferred adapter wins over any score; otherwise discrete,
            // then integrated, then by VRAM.
            bool preferred = !m_Desc.PreferredAdapter.Empty()
                          && name == m_Desc.PreferredAdapter;
            u64  score     = info.DedicatedMemory / (1024 * 1024);
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1ull << 40;
            else if (properties.deviceType
                     == vk::PhysicalDeviceType::eIntegratedGpu)
                score += 1ull << 39;
            if (preferred) score += 1ull << 50;

            if (!havePreferred && (preferred || score > bestScore))
            {
                bestScore           = score;
                havePreferred       = preferred;
                m_Physical          = candidate;
                m_Properties        = properties;
                m_Families.Graphics = graphics;
                m_ActiveAdapter     = static_cast<u32>(m_Adapters.Size() - 1);
            }
        }

        if (!m_Physical)
            return Unexpected(
                String("Vulkan: No GPU meets the requirements (Vulkan 1.3, "
                       "dynamic rendering, synchronization2)"));
        if (!m_Desc.PreferredAdapter.Empty() && !havePreferred)
            VtCoreWarnTag("Renderer",
                          "Vulkan: The preferred GPU '{}' isn't available; "
                          "using '{}'",
                          m_Desc.PreferredAdapter, AdapterName());
        return {};
    }

    Expected<void, String> VulkanDevice::CreateLogicalDevice()
    {
        f32                       priority = 1.0f;
        vk::DeviceQueueCreateInfo queueInfo{};
        queueInfo.queueFamilyIndex = m_Families.Graphics.Value();
        queueInfo.queueCount       = 1;
        queueInfo.pQueuePriorities = &priority;

        Vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        m_MemoryBudget
            = HasDeviceExtension(m_Physical, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
        if (m_MemoryBudget)
            extensions.PushBack(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

        // Enable the optional features the GPU has; the RHI reports them.
        vk::PhysicalDeviceFeatures available{};
        m_Physical.getFeatures(&available);
        m_Features.SamplerAnisotropy    = available.samplerAnisotropy;
        m_Features.FillModeNonSolid     = available.fillModeNonSolid;
        m_Features.TextureCompressionBC = available.textureCompressionBC;
        m_Features.TimestampQueries
            = m_Properties.limits.timestampComputeAndGraphics;

        vk::PhysicalDeviceVulkan13Features features13{};
        features13.dynamicRendering = VK_TRUE;
        features13.synchronization2 = VK_TRUE;
        // HLSL `discard` compiles to OpDemoteToHelperInvocation (required
        // by Vulkan 1.3).
        features13.shaderDemoteToHelperInvocation = VK_TRUE;
        vk::PhysicalDeviceFeatures2 features{};
        features.pNext                         = &features13;
        features.features.samplerAnisotropy    = available.samplerAnisotropy;
        features.features.fillModeNonSolid     = available.fillModeNonSolid;
        features.features.textureCompressionBC = available.textureCompressionBC;
        features.features.geometryShader       = available.geometryShader;
        features.features.independentBlend     = available.independentBlend;

        vk::DeviceCreateInfo info{};
        info.pNext                   = &features;
        info.queueCreateInfoCount    = 1;
        info.pQueueCreateInfos       = &queueInfo;
        info.enabledExtensionCount   = extensions.Size();
        info.ppEnabledExtensionNames = extensions.Raw();

        vk::Result result = m_Physical.createDevice(&info, nullptr, &m_Device);
        if (result != vk::Result::eSuccess)
            return Unexpected(Prism::Format("Vulkan: vkCreateDevice failed: {}",
                                            VkResultToString(result)));
        // Device-level entry points, skipping the loader's trampolines.
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Device);
        m_Device.getQueue(m_Families.Graphics.Value(), 0, &m_GraphicsQueue);
        SetDebugName(m_Device, m_GraphicsQueue, "Graphics queue");
        return {};
    }

    void VulkanDevice::CreateAllocator()
    {
        VmaVulkanFunctions functions{};
        functions.vkGetInstanceProcAddr
            = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
        functions.vkGetDeviceProcAddr
            = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo info{};
        info.vulkanApiVersion = kRequiredApiVersion;
        info.physicalDevice   = m_Physical;
        info.device           = m_Device;
        info.instance         = m_Instance.Handle();
        info.pVulkanFunctions = &functions;
        if (m_MemoryBudget)
            info.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        VkCall(vmaCreateAllocator(&info, &m_Allocator));
    }

    void VulkanDevice::QueryLimits()
    {
        const auto& limits                = m_Properties.limits;
        m_Limits.MaxTextureDimension2D    = limits.maxImageDimension2D;
        m_Limits.MaxColorAttachments      = limits.maxColorAttachments;
        m_Limits.MaxPushConstantSize      = limits.maxPushConstantsSize;
        m_Limits.MaxBoundBindGroups       = limits.maxBoundDescriptorSets;
        m_Limits.MinUniformBufferAlignment = limits.minUniformBufferOffsetAlignment;
        m_Limits.MinStorageBufferAlignment = limits.minStorageBufferOffsetAlignment;
        m_Limits.MaxSamplerAnisotropy      = limits.maxSamplerAnisotropy;
        m_Limits.TimestampPeriodNanoseconds = limits.timestampPeriod;
    }

    void VulkanDevice::CreateFrames()
    {
        m_Frames.Resize(m_Desc.FramesInFlight);

        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.flags            = vk::CommandPoolCreateFlagBits::eTransient;
        poolInfo.queueFamilyIndex = GraphicsFamily();
        vk::FenceCreateInfo fenceInfo{};
        // Signaled, so the first BeginFrame of each slot doesn't wait.
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (auto& frame : m_Frames)
        {
            VkCall(m_Device.createCommandPool(&poolInfo, nullptr, &frame.Pool));
            VkCall(m_Device.createCommandPool(&poolInfo, nullptr,
                                              &frame.Uploads.Pool));
            VkCall(m_Device.createFence(&fenceInfo, nullptr, &frame.Fence));
        }
        VkCall(m_Device.createCommandPool(&poolInfo, nullptr,
                                          &m_IdleUploads.Pool));
    }

    VulkanDevice::~VulkanDevice()
    {
        if (!m_Device)
        {
            m_Instance.Destroy();
            return;
        }
        m_Device.waitIdle();
#ifdef TRACY_ENABLE
        if (m_TracyContext)
            tracy::DestroyVkContext(static_cast<tracy::VkCtx*>(m_TracyContext));
        m_TracyContext = nullptr;
#endif

        for (auto& frame : m_Frames)
        {
            frame.Lists.Clear();
            m_Device.destroyCommandPool(frame.Pool);
            m_Device.destroyCommandPool(frame.Uploads.Pool);
            m_Device.destroyFence(frame.Fence);
            DestroyStaging(frame.Uploads.Staging);
        }
        m_Device.destroyCommandPool(m_IdleUploads.Pool);
        m_Device.destroyFence(m_IdleUploadFence);
        DestroyStaging(m_IdleUploads.Staging);

        CollectGarbage(0, true);
        for (auto pool : m_DescriptorPools) m_Device.destroyDescriptorPool(pool);
        m_Device.destroyDescriptorSetLayout(m_EmptySetLayout);
        SavePipelineCache();
        m_Device.destroyPipelineCache(m_PipelineCache);

        // Anything still allocated was leaked by its owner (a Ref kept past
        // Renderer::Shutdown); say so before VMA asserts on it.
        VmaTotalStatistics statistics{};
        vmaCalculateStatistics(m_Allocator, &statistics);
        if (statistics.total.statistics.allocationCount > 0)
            VtCoreError("Vulkan: {} GPU allocations ({} bytes) still alive at "
                        "device destruction: some RHI resource outlived "
                        "Renderer::Shutdown",
                        statistics.total.statistics.allocationCount,
                        statistics.total.statistics.allocationBytes);
        if (statistics.total.statistics.allocationCount > 0)
        {
            // Every live allocation, by the name of the resource it backs.
            char* json = nullptr;
            vmaBuildStatsString(m_Allocator, &json, VK_TRUE);
            std::string_view text(json ? json : "");
            for (usize at = text.find("\"Name\": \""); at != std::string_view::npos;
                 at       = text.find("\"Name\": \"", at + 1))
            {
                usize begin = at + 9;
                usize end   = text.find('"', begin);
                if (end == std::string_view::npos) break;
                std::string_view name = text.substr(begin, end - begin);
                VtCoreError("Vulkan:   leaked '{}'", StringView(name.data(), name.size()));
            }
            vmaFreeStatsString(m_Allocator, json);
        }
        vmaDestroyAllocator(m_Allocator);
        m_Device.destroy();
        m_Instance.Destroy();
    }

    //------------------------------------------------------------------------
    // Resources -------------------------------------------------------------
    //------------------------------------------------------------------------

    ::Ref<Buffer> VulkanDevice::CreateBuffer(const BufferDesc& desc)
    {
        return CreateRef<VulkanBuffer>(desc);
    }
    ::Ref<Texture> VulkanDevice::CreateTexture(const TextureDesc& desc)
    {
        return CreateRef<VulkanTexture>(desc);
    }
    ::Ref<Sampler> VulkanDevice::CreateSampler(const SamplerDesc& desc)
    {
        return CreateRef<VulkanSampler>(desc);
    }
    Expected<::Ref<Shader>, String>
    VulkanDevice::CreateShader(const ShaderDesc& desc)
    {
        auto shader = VulkanShader::Create(desc);
        if (!shader) return Unexpected(shader.Error());
        return ::Ref<Shader>(*shader);
    }
    ::Ref<Pipeline> VulkanDevice::CreatePipeline(const GraphicsPipelineDesc& desc)
    {
        return CreateRef<VulkanPipeline>(desc);
    }
    ::Ref<Pipeline>
    VulkanDevice::CreateComputePipeline(const ComputePipelineDesc& desc)
    {
        return CreateRef<VulkanPipeline>(desc);
    }
    ::Ref<BindGroup> VulkanDevice::CreateBindGroup(const BindGroupDesc& desc)
    {
        return CreateRef<VulkanBindGroup>(desc);
    }
    ::Ref<SwapChain> VulkanDevice::CreateSwapChain(const SwapChainDesc& desc)
    {
        auto swapChain = CreateRef<VulkanSwapChain>(desc);
        if (!swapChain->Valid()) return nullptr;
        return swapChain;
    }

    //------------------------------------------------------------------------
    // Deferred deletion -----------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanDevice::Defer(Deletion type, u64 handle, VmaAllocation allocation,
                             u64 owner)
    {
        if (!handle) return;
        std::lock_guard lock(m_DeletionMutex);
        // Outside a frame, the object may still be used by uploads or by the
        // next frame's submissions recorded earlier; give it one more frame.
        u64 frame = m_FrameNumber + (m_InFrame ? 0 : 1);
        m_Deletions.PushBack({type, handle, owner, allocation, frame});
    }

    void VulkanDevice::CollectGarbage(u64 completedFrame, bool all)
    {
        Vector<DeferredDeletion> ready;
        {
            std::lock_guard lock(m_DeletionMutex);
            for (usize i = 0; i < m_Deletions.Size();)
            {
                if (all || m_Deletions[i].Frame <= completedFrame)
                {
                    ready.PushBack(m_Deletions[i]);
                    m_Deletions[i] = m_Deletions.Back();
                    m_Deletions.PopBack();
                }
                else ++i;
            }
        }
        // Dependents first: swapchains before their surfaces, views and
        // sets before what they reference (the enum is in that order).
        std::sort(ready.begin(), ready.end(),
                  [](const DeferredDeletion& a, const DeferredDeletion& b)
                  { return DestroyOrder(a.Kind) < DestroyOrder(b.Kind); });
        for (const auto& entry : ready) DestroyNow(entry);
    }

    void VulkanDevice::DestroyNow(const DeferredDeletion& entry)
    {
        auto handle = [&]<typename T>() { return reinterpret_cast<T>(entry.Handle); };
        switch (entry.Kind)
        {
            case Deletion::eBuffer:
                vmaDestroyBuffer(m_Allocator, handle.operator()<VkBuffer>(),
                                 entry.Allocation);
                break;
            case Deletion::eImage:
                vmaDestroyImage(m_Allocator, handle.operator()<VkImage>(),
                                entry.Allocation);
                break;
            case Deletion::eImageView:
                m_Device.destroyImageView(handle.operator()<VkImageView>());
                break;
            case Deletion::eSampler:
                m_Device.destroySampler(handle.operator()<VkSampler>());
                break;
            case Deletion::ePipeline:
                m_Device.destroyPipeline(handle.operator()<VkPipeline>());
                break;
            case Deletion::ePipelineLayout:
                m_Device.destroyPipelineLayout(
                    handle.operator()<VkPipelineLayout>());
                break;
            case Deletion::eDescriptorSetLayout:
                m_Device.destroyDescriptorSetLayout(
                    handle.operator()<VkDescriptorSetLayout>());
                break;
            case Deletion::eDescriptorSet:
            {
                std::lock_guard   lock(m_DescriptorMutex);
                vk::DescriptorSet set(handle.operator()<VkDescriptorSet>());
                VkCall(m_Device.freeDescriptorSets(
                    reinterpret_cast<VkDescriptorPool>(entry.Owner), 1, &set));
                break;
            }
            case Deletion::eShaderModule:
                m_Device.destroyShaderModule(handle.operator()<VkShaderModule>());
                break;
            case Deletion::eSwapChain:
                m_Device.destroySwapchainKHR(handle.operator()<VkSwapchainKHR>());
                break;
            case Deletion::eSurface:
                m_Instance.Handle().destroySurfaceKHR(
                    handle.operator()<VkSurfaceKHR>());
                break;
            case Deletion::eSemaphore:
                m_Device.destroySemaphore(handle.operator()<VkSemaphore>());
                break;
        }
    }

    //------------------------------------------------------------------------
    // Descriptors -----------------------------------------------------------
    //------------------------------------------------------------------------

    vk::DescriptorSet
    VulkanDevice::AllocateDescriptorSet(vk::DescriptorSetLayout layout,
                                        vk::DescriptorPool&     outPool)
    {
        std::lock_guard lock(m_DescriptorMutex);

        vk::DescriptorSetAllocateInfo info{};
        info.descriptorSetCount = 1;
        info.pSetLayouts        = &layout;
        vk::DescriptorSet set   = VK_NULL_HANDLE;

        // Newest pool first: older ones are mostly full.
        for (usize i = m_DescriptorPools.Size(); i-- > 0;)
        {
            info.descriptorPool = m_DescriptorPools[i];
            vk::Result result   = m_Device.allocateDescriptorSets(&info, &set);
            if (result == vk::Result::eSuccess)
            {
                outPool = m_DescriptorPools[i];
                return set;
            }
            if (result != vk::Result::eErrorOutOfPoolMemory
                && result != vk::Result::eErrorFragmentedPool)
                VkCall(result);
        }

        constexpr vk::DescriptorType types[] = {
            vk::DescriptorType::eUniformBuffer,
            vk::DescriptorType::eStorageBuffer,
            vk::DescriptorType::eCombinedImageSampler,
            vk::DescriptorType::eSampledImage,
            vk::DescriptorType::eSampler,
            vk::DescriptorType::eStorageImage,
        };
        Vector<vk::DescriptorPoolSize> sizes;
        for (auto type : types)
            sizes.PushBack(vk::DescriptorPoolSize(type, kDescriptorPoolSets * 4));

        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.maxSets = kDescriptorPoolSets;
        poolInfo.poolSizeCount = sizes.Size();
        poolInfo.pPoolSizes    = sizes.Raw();
        vk::DescriptorPool pool = VK_NULL_HANDLE;
        VkCall(m_Device.createDescriptorPool(&poolInfo, nullptr, &pool));
        m_DescriptorPools.PushBack(pool);

        info.descriptorPool = pool;
        VkCall(m_Device.allocateDescriptorSets(&info, &set));
        outPool = pool;
        return set;
    }

    //------------------------------------------------------------------------
    // Uploads ---------------------------------------------------------------
    //------------------------------------------------------------------------

    VulkanDevice::UploadContext& VulkanDevice::CurrentUploads()
    {
        return m_InFrame ? m_Frames[m_FrameIndex].Uploads : m_IdleUploads;
    }

    vk::CommandBuffer VulkanDevice::BeginUploads(UploadContext& context)
    {
        if (context.Recording) return context.Buffer;

        // The idle context is reused as soon as its last batch finished.
        if (&context == &m_IdleUploads && m_IdleUploadsInFlight)
        {
            VkCall(m_Device.waitForFences(1, &m_IdleUploadFence, VK_TRUE,
                                          std::numeric_limits<u64>::max()));
            VkCall(m_Device.resetFences(1, &m_IdleUploadFence));
            m_IdleUploadsInFlight = false;
            m_Device.resetCommandPool(context.Pool);
            context.Buffer = VK_NULL_HANDLE;
            ResetStaging(context.Staging);
        }

        if (!context.Buffer)
        {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool        = context.Pool;
            allocInfo.level              = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;
            VkCall(m_Device.allocateCommandBuffers(&allocInfo, &context.Buffer));
        }
        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        VkCall(context.Buffer.begin(&beginInfo));
        context.Recording = true;
        return context.Buffer;
    }

    vk::CommandBuffer VulkanDevice::EndUploads(UploadContext& context)
    {
        if (!context.Recording) return VK_NULL_HANDLE;

        // Make every upload visible to whatever runs after it on the queue.
        vk::MemoryBarrier2 barrier{};
        barrier.srcStageMask  = vk::PipelineStageFlagBits2::eAllTransfer;
        barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
        barrier.dstStageMask  = vk::PipelineStageFlagBits2::eAllCommands;
        barrier.dstAccessMask
            = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
        vk::DependencyInfo dependency{};
        dependency.memoryBarrierCount = 1;
        dependency.pMemoryBarriers    = &barrier;
        context.Buffer.pipelineBarrier2(&dependency);

        context.Buffer.end();
        context.Recording = false;
        return context.Buffer;
    }

    void VulkanDevice::FlushIdleUploads()
    {
        std::lock_guard   lock(m_UploadMutex);
        vk::CommandBuffer buffer = EndUploads(m_IdleUploads);
        if (!buffer) return;

        vk::CommandBufferSubmitInfo bufferInfo{};
        bufferInfo.commandBuffer = buffer;
        vk::SubmitInfo2 submit{};
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos    = &bufferInfo;
        VkCall(m_GraphicsQueue.submit2(1, &submit, m_IdleUploadFence));
        m_IdleUploadsInFlight = true;
    }

    VulkanDevice::StagingAllocation
    VulkanDevice::AllocateStaging(StagingRing& ring, usize size, usize alignment)
    {
        while (ring.Chunk < ring.Chunks.Size())
        {
            auto& chunk  = ring.Chunks[ring.Chunk];
            usize offset = AlignUp(ring.Offset, alignment);
            if (offset + size <= chunk.Size)
            {
                ring.Offset = offset + size;
                return {chunk.Buffer, offset, chunk.Mapped + offset};
            }
            ++ring.Chunk;
            ring.Offset = 0;
        }

        StagingChunk chunk{};
        chunk.Size = size > kStagingChunkSize ? AlignUp(size, 4096)
                                              : kStagingChunkSize;
        vk::BufferCreateInfo info{};
        info.size  = chunk.Size;
        info.usage = vk::BufferUsageFlagBits::eTransferSrc;
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                        | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VmaAllocationInfo result{};
        VkCall(vmaCreateBuffer(m_Allocator,
                               reinterpret_cast<VkBufferCreateInfo*>(&info),
                               &allocInfo,
                               reinterpret_cast<VkBuffer*>(&chunk.Buffer),
                               &chunk.Allocation, &result));
        chunk.Mapped = static_cast<u8*>(result.pMappedData);
        ring.Chunks.PushBack(chunk);
        ring.Chunk  = ring.Chunks.Size() - 1;
        ring.Offset = size;
        return {chunk.Buffer, 0, chunk.Mapped};
    }

    void VulkanDevice::ResetStaging(StagingRing& ring)
    {
        // Oversized chunks served one big upload; don't keep them around.
        for (usize i = 0; i < ring.Chunks.Size();)
        {
            if (ring.Chunks[i].Size > kStagingChunkSize)
            {
                vmaDestroyBuffer(m_Allocator, ring.Chunks[i].Buffer,
                                 ring.Chunks[i].Allocation);
                ring.Chunks[i] = ring.Chunks.Back();
                ring.Chunks.PopBack();
            }
            else ++i;
        }
        ring.Chunk  = 0;
        ring.Offset = 0;
    }

    void VulkanDevice::DestroyStaging(StagingRing& ring)
    {
        for (auto& chunk : ring.Chunks)
            vmaDestroyBuffer(m_Allocator, chunk.Buffer, chunk.Allocation);
        ring.Chunks.Clear();
        ring.Chunk  = 0;
        ring.Offset = 0;
    }

    void VulkanDevice::UploadBuffer(Buffer& buffer, const void* data, usize size,
                                    usize offset)
    {
        VtCoreAssertMsg(offset + size <= buffer.Size(),
                        "Vulkan: UploadBuffer out of bounds");
        VtCoreAssertMsg(HasAny(buffer.Desc().Usage, BufferUsage::eTransferDst),
                        "Vulkan: UploadBuffer needs a buffer with TransferDst "
                        "usage");
        if (size == 0) return;

        std::lock_guard   lock(m_UploadMutex);
        auto&             context = CurrentUploads();
        vk::CommandBuffer cmd     = BeginUploads(context);
        auto              staging = AllocateStaging(context.Staging, size, 16);
        std::memcpy(staging.Mapped, data, size);

        vk::BufferCopy copy(staging.Offset, offset, size);
        cmd.copyBuffer(staging.Buffer, static_cast<VulkanBuffer&>(buffer).Handle(),
                       1, &copy);
    }

    void VulkanDevice::UploadTexture(Texture& texture, const void* data,
                                     usize size, const TextureRegion& region,
                                     ResourceState finalState)
    {
        auto&    vkTexture = static_cast<VulkanTexture&>(texture);
        Extent3D extent    = region.Extent;
        if (extent.Width == 0)
        {
            auto mipSize = [](u32 s, u32 mip) { return (s >> mip) ? (s >> mip) : 1u; };
            extent = {mipSize(texture.Width(), region.MipLevel),
                      mipSize(texture.Height(), region.MipLevel),
                      mipSize(texture.Extent().Depth, region.MipLevel)};
        }
        usize required = FormatSize(texture.Format(), extent);
        VtCoreAssertFormat(size >= required,
                           "Vulkan: UploadTexture got {} bytes, the region "
                           "needs {}",
                           size, required);
        VtCoreAssertMsg(HasAny(texture.Desc().Usage, TextureUsage::eTransferDst),
                        "Vulkan: UploadTexture needs a texture with "
                        "TransferDst usage");

        std::lock_guard   lock(m_UploadMutex);
        auto&             context = CurrentUploads();
        vk::CommandBuffer cmd     = BeginUploads(context);
        usize             texel   = GetFormatInfo(texture.Format()).BytesPerBlock;
        auto staging = AllocateStaging(context.Staging, required,
                                       texel > 16 ? texel : 16);
        std::memcpy(staging.Mapped, data, required);

        TextureBarrier toCopy{&texture, texture.TrackedState(),
                              ResourceState::eTransferDst};
        VulkanCommandList::RecordBarrier(cmd, Span<const TextureBarrier>(&toCopy, 1),
                                         {});

        vk::BufferImageCopy copy{};
        copy.bufferOffset                    = staging.Offset;
        copy.imageSubresource.aspectMask     = vkTexture.Aspect();
        copy.imageSubresource.mipLevel       = region.MipLevel;
        copy.imageSubresource.baseArrayLayer = region.Layer;
        copy.imageSubresource.layerCount     = 1;
        copy.imageOffset = vk::Offset3D(static_cast<i32>(region.X),
                                        static_cast<i32>(region.Y),
                                        static_cast<i32>(region.Z));
        copy.imageExtent = vk::Extent3D(extent.Width, extent.Height, extent.Depth);
        cmd.copyBufferToImage(staging.Buffer, vkTexture.Handle(),
                              vk::ImageLayout::eTransferDstOptimal, 1, &copy);

        if (finalState != ResourceState::eTransferDst)
        {
            TextureBarrier toFinal{&texture, ResourceState::eTransferDst,
                                   finalState};
            VulkanCommandList::RecordBarrier(
                cmd, Span<const TextureBarrier>(&toFinal, 1), {});
        }
    }

    void VulkanDevice::GenerateMips(Texture& texture, ResourceState finalState)
    {
        VtCoreAssertMsg(HasAll(texture.Desc().Usage,
                               TextureUsage::eTransferSrc
                                   | TextureUsage::eTransferDst),
                        "Vulkan: GenerateMips needs TransferSrc and "
                        "TransferDst usage");
        std::lock_guard lock(m_UploadMutex);
        vk::CommandBuffer cmd = BeginUploads(CurrentUploads());
        VulkanCommandList::RecordGenerateMips(
            cmd, static_cast<VulkanTexture&>(texture), finalState);
    }

    //------------------------------------------------------------------------
    // Frames ----------------------------------------------------------------
    //------------------------------------------------------------------------

    void VulkanDevice::CheckRenderThread()
    {
#ifdef VT_DEBUG
        if (m_RenderThread == std::thread::id{})
            m_RenderThread = std::this_thread::get_id();
        VtCoreAssertMsg(m_RenderThread == std::this_thread::get_id(),
                        "Vulkan: Frame and recording calls must come from one "
                        "thread (the render thread)");
#endif
    }

    void VulkanDevice::BeginFrame()
    {
        CheckRenderThread();
        VtCoreAssertMsg(!m_InFrame, "Vulkan: BeginFrame inside a frame");

        // Uploads made since the last frame (asset loading) go first.
        FlushIdleUploads();

        ++m_FrameNumber;
        m_FrameIndex = static_cast<u32>(m_FrameNumber % m_Desc.FramesInFlight);
        Frame& frame = m_Frames[m_FrameIndex];

        // Wait for the frame that last used this slot.
        VkCall(m_Device.waitForFences(1, &frame.Fence, VK_TRUE,
                                      std::numeric_limits<u64>::max()));
        VkCall(m_Device.resetFences(1, &frame.Fence));

        // Every frame up to that one is complete, and so are the idle uploads
        // submitted before it.
        if (m_IdleUploadsInFlight
            && m_Device.getFenceStatus(m_IdleUploadFence) == vk::Result::eSuccess)
        {
            std::lock_guard lock(m_UploadMutex);
            VkCall(m_Device.resetFences(1, &m_IdleUploadFence));
            m_IdleUploadsInFlight = false;
            m_Device.resetCommandPool(m_IdleUploads.Pool);
            m_IdleUploads.Buffer = VK_NULL_HANDLE;
            ResetStaging(m_IdleUploads.Staging);
        }
        if (m_FrameNumber > m_Desc.FramesInFlight && !m_IdleUploadsInFlight)
            CollectGarbage(m_FrameNumber - m_Desc.FramesInFlight);

        m_Device.resetCommandPool(frame.Pool);
        frame.ListsUsed  = 0;
        m_TracyCollected = false;
        {
            std::lock_guard lock(m_UploadMutex);
            m_Device.resetCommandPool(frame.Uploads.Pool);
            frame.Uploads.Buffer    = VK_NULL_HANDLE;
            frame.Uploads.Recording = false;
            ResetStaging(frame.Uploads.Staging);
            m_InFrame = true;
        }
    }

    RHI::CommandList& VulkanDevice::BeginCommandList(QueueType queue)
    {
        CheckRenderThread();
        VtCoreAssertMsg(m_InFrame,
                        "Vulkan: BeginCommandList outside BeginFrame/EndFrame");
        Frame& frame = m_Frames[m_FrameIndex];

        if (frame.ListsUsed == frame.Lists.Size())
        {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool        = frame.Pool;
            allocInfo.level              = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandBufferCount = 1;
            vk::CommandBuffer buffer     = VK_NULL_HANDLE;
            VkCall(m_Device.allocateCommandBuffers(&allocInfo, &buffer));
            // A single queue serves every queue type for now.
            frame.Lists.PushBack(CreateScope<VulkanCommandList>(queue, buffer));
        }
        auto& list = *frame.Lists[frame.ListsUsed++];
        list.Begin();
#ifdef TRACY_ENABLE
        if (m_TracyContext && !m_TracyCollected)
        {
            TracyVkCollect(static_cast<tracy::VkCtx*>(m_TracyContext), list.Handle());
            m_TracyCollected = true;
        }
#endif
        return list;
    }

    void VulkanDevice::Submit(RHI::CommandList&            commands,
                              Span<const ::Ref<SwapChain>> present)
    {
        CheckRenderThread();
        VtCoreAssertMsg(m_InFrame, "Vulkan: Submit outside BeginFrame/EndFrame");
        auto& list = static_cast<VulkanCommandList&>(commands);

        Vector<VulkanSwapChain*>           swapChains;
        Vector<vk::SemaphoreSubmitInfo>    waits;
        Vector<vk::SemaphoreSubmitInfo>    signals;
        for (const auto& swapChain : present)
        {
            VtCoreAssertMsg(swapChain, "Vulkan: Presenting a null swapchain");
            auto* vkSwapChain = static_cast<VulkanSwapChain*>(swapChain.Raw());
            VtCoreAssertMsg(vkSwapChain->HasAcquiredImage(),
                            "Vulkan: Presenting a swapchain without an "
                            "acquired image");
            swapChains.PushBack(vkSwapChain);

            // Presentation needs the image in ePresent.
            list.Transition(*vkSwapChain->CurrentImage(), ResourceState::ePresent);

            vk::SemaphoreSubmitInfo& wait = waits.EmplaceBack();
            wait.semaphore = vkSwapChain->AcquireSemaphore();
            wait.stageMask = vk::PipelineStageFlagBits2::eAllCommands;
            vk::SemaphoreSubmitInfo& signal = signals.EmplaceBack();
            signal.semaphore = vkSwapChain->PresentSemaphore();
            signal.stageMask = vk::PipelineStageFlagBits2::eAllCommands;
        }
        list.End();

        Vector<vk::CommandBufferSubmitInfo> buffers;
        {
            std::lock_guard   lock(m_UploadMutex);
            vk::CommandBuffer uploads = EndUploads(m_Frames[m_FrameIndex].Uploads);
            if (uploads) buffers.EmplaceBack().commandBuffer = uploads;
        }
        buffers.EmplaceBack().commandBuffer = list.Handle();

        vk::SubmitInfo2 submit{};
        submit.waitSemaphoreInfoCount   = waits.Size();
        submit.pWaitSemaphoreInfos      = waits.Raw();
        submit.commandBufferInfoCount   = buffers.Size();
        submit.pCommandBufferInfos      = buffers.Raw();
        submit.signalSemaphoreInfoCount = signals.Size();
        submit.pSignalSemaphoreInfos    = signals.Raw();
        VkCall(m_GraphicsQueue.submit2(1, &submit, VK_NULL_HANDLE));

        if (swapChains.Empty()) return;

        Vector<vk::SwapchainKHR> handles;
        Vector<u32>              indices;
        Vector<vk::Semaphore>    presentWaits;
        Vector<vk::Result>       results(swapChains.Size());
        for (auto* swapChain : swapChains)
        {
            handles.PushBack(swapChain->Handle());
            indices.PushBack(swapChain->ImageIndex());
            presentWaits.PushBack(swapChain->PresentSemaphore());
        }
        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = presentWaits.Size();
        presentInfo.pWaitSemaphores    = presentWaits.Raw();
        presentInfo.swapchainCount     = handles.Size();
        presentInfo.pSwapchains        = handles.Raw();
        presentInfo.pImageIndices      = indices.Raw();
        presentInfo.pResults           = results.Raw();
        // Out-of-date results come back per swapchain; ignore the summary.
        (void)m_GraphicsQueue.presentKHR(&presentInfo);
        for (usize i = 0; i < swapChains.Size(); ++i)
            swapChains[i]->OnPresented(results[i]);
    }

    void VulkanDevice::EndFrame()
    {
        CheckRenderThread();
        VtCoreAssertMsg(m_InFrame, "Vulkan: EndFrame without BeginFrame");
        Frame& frame = m_Frames[m_FrameIndex];

        // Signal the frame fence after everything submitted this frame,
        // including uploads nobody submitted a command list after.
        vk::CommandBufferSubmitInfo uploads{};
        vk::SubmitInfo2             submit{};
        {
            std::lock_guard lock(m_UploadMutex);
            uploads.commandBuffer = EndUploads(frame.Uploads);
            m_InFrame             = false;
        }
        if (uploads.commandBuffer)
        {
            submit.commandBufferInfoCount = 1;
            submit.pCommandBufferInfos    = &uploads;
        }
        VkCall(m_GraphicsQueue.submit2(1, &submit, frame.Fence));
    }

    void VulkanDevice::ExecuteImmediate(Delegate<void(RHI::CommandList&)> commands)
    {
        VtCoreAssertMsg(!m_InFrame,
                        "Vulkan: ExecuteImmediate inside a frame; it is for "
                        "load-time work");

        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.flags            = vk::CommandPoolCreateFlagBits::eTransient;
        poolInfo.queueFamilyIndex = GraphicsFamily();
        vk::CommandPool pool      = VK_NULL_HANDLE;
        VkCall(m_Device.createCommandPool(&poolInfo, nullptr, &pool));

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.commandPool        = pool;
        allocInfo.level              = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;
        vk::CommandBuffer buffer     = VK_NULL_HANDLE;
        VkCall(m_Device.allocateCommandBuffers(&allocInfo, &buffer));

        VulkanCommandList list(QueueType::eGraphics, buffer);
        list.Begin();
        commands(list);
        list.End();

        // Uploads recorded so far go in the same batch, first.
        Vector<vk::CommandBufferSubmitInfo> buffers;
        {
            std::lock_guard lock(m_UploadMutex);
            // An earlier idle batch still owns the fence: let it finish.
            if (m_IdleUploadsInFlight && !m_IdleUploads.Recording)
            {
                VkCall(m_Device.waitForFences(1, &m_IdleUploadFence, VK_TRUE,
                                              std::numeric_limits<u64>::max()));
                VkCall(m_Device.resetFences(1, &m_IdleUploadFence));
                m_IdleUploadsInFlight = false;
                m_Device.resetCommandPool(m_IdleUploads.Pool);
                m_IdleUploads.Buffer = VK_NULL_HANDLE;
                ResetStaging(m_IdleUploads.Staging);
            }
            vk::CommandBuffer uploads = EndUploads(m_IdleUploads);
            if (uploads) buffers.EmplaceBack().commandBuffer = uploads;
        }
        buffers.EmplaceBack().commandBuffer = buffer;

        vk::SubmitInfo2 submit{};
        submit.commandBufferInfoCount = buffers.Size();
        submit.pCommandBufferInfos    = buffers.Raw();
        // The idle fence also covers the uploads, so the idle context can be
        // reused afterwards.
        VkCall(m_GraphicsQueue.submit2(1, &submit, m_IdleUploadFence));
        VkCall(m_Device.waitForFences(1, &m_IdleUploadFence, VK_TRUE,
                                      std::numeric_limits<u64>::max()));
        {
            std::lock_guard lock(m_UploadMutex);
            VkCall(m_Device.resetFences(1, &m_IdleUploadFence));
            m_IdleUploadsInFlight = false;
            m_Device.resetCommandPool(m_IdleUploads.Pool);
            m_IdleUploads.Buffer = VK_NULL_HANDLE;
            ResetStaging(m_IdleUploads.Staging);
        }
        m_Device.destroyCommandPool(pool);
    }

    void VulkanDevice::WaitIdle()
    {
        m_Device.waitIdle();
    }

    //------------------------------------------------------------------------
    // Queries ---------------------------------------------------------------
    //------------------------------------------------------------------------

    MemoryStats VulkanDevice::QueryMemoryStats() const
    {
        MemoryStats stats{};
        VmaBudget   budgets[VK_MAX_MEMORY_HEAPS]{};
        vmaGetHeapBudgets(m_Allocator, budgets);

        vk::PhysicalDeviceMemoryProperties memory{};
        m_Physical.getMemoryProperties(&memory);
        for (u32 i = 0; i < memory.memoryHeapCount; ++i)
        {
            if (!(memory.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal))
                continue;
            stats.DeviceLocalUsage += budgets[i].usage;
            stats.DeviceLocalBudget += budgets[i].budget;
        }
        return stats;
    }

    String VulkanDevice::AdapterName() const
    {
        return String(m_Properties.deviceName.data());
    }
} // namespace Vortex::Vulkan

// Declared in the Vortex.RHI.Vulkan module, attached to the global module
// (extern "C++") so it can be defined here, next to the backend's headers.
VT_RENDERER_RHI_VULKAN_API void Vortex::Vulkan::RegisterBackend()
{
    RHI::Device::RegisterAPI(RHI::API::eVulkan, &VulkanDevice::Create);
}
