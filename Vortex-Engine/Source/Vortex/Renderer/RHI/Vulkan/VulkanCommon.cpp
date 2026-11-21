/*
 * Created by v1tr10l7 on 13.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

namespace Vortex::Vulkan
{
    using namespace RHI;

    String VkResultToString(vk::Result result)
    {
        auto str = vk::to_string(result);
        return String(str.data(), str.size());
    }

    //------------------------------------------------------------------------
    // Formats ---------------------------------------------------------------
    //------------------------------------------------------------------------

    namespace
    {
        struct FormatPair
        {
            RHI::Format RHI;
            vk::Format  Vk;
        };
        constexpr FormatPair s_Formats[] = {
            {Format::eUndefined, vk::Format::eUndefined},
            {Format::eR8Unorm, vk::Format::eR8Unorm},
            {Format::eR8Uint, vk::Format::eR8Uint},
            {Format::eRG8Unorm, vk::Format::eR8G8Unorm},
            {Format::eRGBA8Unorm, vk::Format::eR8G8B8A8Unorm},
            {Format::eRGBA8Srgb, vk::Format::eR8G8B8A8Srgb},
            {Format::eRGBA8Uint, vk::Format::eR8G8B8A8Uint},
            {Format::eBGRA8Unorm, vk::Format::eB8G8R8A8Unorm},
            {Format::eBGRA8Srgb, vk::Format::eB8G8R8A8Srgb},
            {Format::eR16Uint, vk::Format::eR16Uint},
            {Format::eR16Float, vk::Format::eR16Sfloat},
            {Format::eRG16Float, vk::Format::eR16G16Sfloat},
            {Format::eRGBA16Float, vk::Format::eR16G16B16A16Sfloat},
            {Format::eR32Uint, vk::Format::eR32Uint},
            {Format::eR32Sint, vk::Format::eR32Sint},
            {Format::eR32Float, vk::Format::eR32Sfloat},
            {Format::eRG32Uint, vk::Format::eR32G32Uint},
            {Format::eRG32Sint, vk::Format::eR32G32Sint},
            {Format::eRG32Float, vk::Format::eR32G32Sfloat},
            {Format::eRGB32Uint, vk::Format::eR32G32B32Uint},
            {Format::eRGB32Sint, vk::Format::eR32G32B32Sint},
            {Format::eRGB32Float, vk::Format::eR32G32B32Sfloat},
            {Format::eRGBA32Uint, vk::Format::eR32G32B32A32Uint},
            {Format::eRGBA32Sint, vk::Format::eR32G32B32A32Sint},
            {Format::eRGBA32Float, vk::Format::eR32G32B32A32Sfloat},
            {Format::eRGB10A2Unorm, vk::Format::eA2B10G10R10UnormPack32},
            {Format::eRG11B10Float, vk::Format::eB10G11R11UfloatPack32},
            {Format::eD16Unorm, vk::Format::eD16Unorm},
            {Format::eD32Float, vk::Format::eD32Sfloat},
            {Format::eD24UnormS8Uint, vk::Format::eD24UnormS8Uint},
            {Format::eD32FloatS8Uint, vk::Format::eD32SfloatS8Uint},
            {Format::eBC1RGBAUnorm, vk::Format::eBc1RgbaUnormBlock},
            {Format::eBC1RGBASrgb, vk::Format::eBc1RgbaSrgbBlock},
            {Format::eBC3RGBAUnorm, vk::Format::eBc3UnormBlock},
            {Format::eBC3RGBASrgb, vk::Format::eBc3SrgbBlock},
            {Format::eBC4RUnorm, vk::Format::eBc4UnormBlock},
            {Format::eBC5RGUnorm, vk::Format::eBc5UnormBlock},
            {Format::eBC6HRGBFloat, vk::Format::eBc6HUfloatBlock},
            {Format::eBC7RGBAUnorm, vk::Format::eBc7UnormBlock},
            {Format::eBC7RGBASrgb, vk::Format::eBc7SrgbBlock},
        };
        static_assert(sizeof(s_Formats) / sizeof(s_Formats[0])
                          == static_cast<usize>(Format::eCount),
                      "Vulkan: every RHI::Format needs a vk::Format");
    } // namespace

    vk::Format ToVk(RHI::Format format)
    {
        for (const auto& pair : s_Formats)
            if (pair.RHI == format) return pair.Vk;
        return vk::Format::eUndefined;
    }
    RHI::Format FromVk(vk::Format format)
    {
        for (const auto& pair : s_Formats)
            if (pair.Vk == format) return pair.RHI;
        return Format::eUndefined;
    }

    vk::ImageAspectFlags AspectOf(RHI::Format format)
    {
        FormatInfo info = GetFormatInfo(format);
        if (!info.Depth) return vk::ImageAspectFlagBits::eColor;
        vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth;
        if (info.Stencil) aspect |= vk::ImageAspectFlagBits::eStencil;
        return aspect;
    }

    //------------------------------------------------------------------------
    // Usages ----------------------------------------------------------------
    //------------------------------------------------------------------------

    vk::BufferUsageFlags ToVk(BufferUsage usage)
    {
        vk::BufferUsageFlags flags{};
        if (HasAny(usage, BufferUsage::eVertex))
            flags |= vk::BufferUsageFlagBits::eVertexBuffer;
        if (HasAny(usage, BufferUsage::eIndex))
            flags |= vk::BufferUsageFlagBits::eIndexBuffer;
        if (HasAny(usage, BufferUsage::eUniform))
            flags |= vk::BufferUsageFlagBits::eUniformBuffer;
        if (HasAny(usage, BufferUsage::eStorage))
            flags |= vk::BufferUsageFlagBits::eStorageBuffer;
        if (HasAny(usage, BufferUsage::eIndirect))
            flags |= vk::BufferUsageFlagBits::eIndirectBuffer;
        if (HasAny(usage, BufferUsage::eTransferSrc))
            flags |= vk::BufferUsageFlagBits::eTransferSrc;
        if (HasAny(usage, BufferUsage::eTransferDst))
            flags |= vk::BufferUsageFlagBits::eTransferDst;
        return flags;
    }

    vk::ImageUsageFlags ToVk(TextureUsage usage)
    {
        vk::ImageUsageFlags flags{};
        if (HasAny(usage, TextureUsage::eSampled))
            flags |= vk::ImageUsageFlagBits::eSampled;
        if (HasAny(usage, TextureUsage::eStorage))
            flags |= vk::ImageUsageFlagBits::eStorage;
        if (HasAny(usage, TextureUsage::eColorAttachment))
            flags |= vk::ImageUsageFlagBits::eColorAttachment;
        if (HasAny(usage, TextureUsage::eDepthStencilAttachment))
            flags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        if (HasAny(usage, TextureUsage::eTransferSrc))
            flags |= vk::ImageUsageFlagBits::eTransferSrc;
        if (HasAny(usage, TextureUsage::eTransferDst))
            flags |= vk::ImageUsageFlagBits::eTransferDst;
        return flags;
    }

    vk::ShaderStageFlags ToVk(ShaderStage stages)
    {
        vk::ShaderStageFlags flags{};
        if (HasAny(stages, ShaderStage::eVertex))
            flags |= vk::ShaderStageFlagBits::eVertex;
        if (HasAny(stages, ShaderStage::eFragment))
            flags |= vk::ShaderStageFlagBits::eFragment;
        if (HasAny(stages, ShaderStage::eCompute))
            flags |= vk::ShaderStageFlagBits::eCompute;
        if (HasAny(stages, ShaderStage::eGeometry))
            flags |= vk::ShaderStageFlagBits::eGeometry;
        if (HasAny(stages, ShaderStage::eTessControl))
            flags |= vk::ShaderStageFlagBits::eTessellationControl;
        if (HasAny(stages, ShaderStage::eTessEval))
            flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
        return flags;
    }
    vk::ShaderStageFlagBits ToVkStage(ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage::eVertex: return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::eFragment:
                return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::eCompute:
                return vk::ShaderStageFlagBits::eCompute;
            case ShaderStage::eGeometry:
                return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::eTessControl:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::eTessEval:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            default: break;
        }
        VtCoreAssertMsg(false, "Vulkan: Not a single shader stage");
        return vk::ShaderStageFlagBits::eVertex;
    }
    RHI::ShaderStage FromVk(vk::ShaderStageFlags stages)
    {
        ShaderStage result = ShaderStage::eNone;
        if (stages & vk::ShaderStageFlagBits::eVertex)
            result |= ShaderStage::eVertex;
        if (stages & vk::ShaderStageFlagBits::eFragment)
            result |= ShaderStage::eFragment;
        if (stages & vk::ShaderStageFlagBits::eCompute)
            result |= ShaderStage::eCompute;
        if (stages & vk::ShaderStageFlagBits::eGeometry)
            result |= ShaderStage::eGeometry;
        if (stages & vk::ShaderStageFlagBits::eTessellationControl)
            result |= ShaderStage::eTessControl;
        if (stages & vk::ShaderStageFlagBits::eTessellationEvaluation)
            result |= ShaderStage::eTessEval;
        return result;
    }

    vk::DescriptorType ToVk(BindingType type)
    {
        switch (type)
        {
            case BindingType::eUniformBuffer:
                return vk::DescriptorType::eUniformBuffer;
            case BindingType::eStorageBuffer:
                return vk::DescriptorType::eStorageBuffer;
            case BindingType::eSampledTexture:
                return vk::DescriptorType::eCombinedImageSampler;
            case BindingType::eTexture:
                return vk::DescriptorType::eSampledImage;
            case BindingType::eSampler: return vk::DescriptorType::eSampler;
            case BindingType::eStorageTexture:
                return vk::DescriptorType::eStorageImage;
        }
        return vk::DescriptorType::eUniformBuffer;
    }

    //------------------------------------------------------------------------
    // Fixed-function state --------------------------------------------------
    //------------------------------------------------------------------------

    vk::PrimitiveTopology ToVk(Topology topology)
    {
        switch (topology)
        {
            case Topology::eTriangleList:
                return vk::PrimitiveTopology::eTriangleList;
            case Topology::eTriangleStrip:
                return vk::PrimitiveTopology::eTriangleStrip;
            case Topology::eLineList: return vk::PrimitiveTopology::eLineList;
            case Topology::eLineStrip: return vk::PrimitiveTopology::eLineStrip;
            case Topology::ePointList: return vk::PrimitiveTopology::ePointList;
        }
        return vk::PrimitiveTopology::eTriangleList;
    }
    vk::PolygonMode ToVk(RHI::PolygonMode mode)
    {
        switch (mode)
        {
            case PolygonMode::eFill: return vk::PolygonMode::eFill;
            case PolygonMode::eLine: return vk::PolygonMode::eLine;
            case PolygonMode::ePoint: return vk::PolygonMode::ePoint;
        }
        return vk::PolygonMode::eFill;
    }
    vk::CullModeFlags ToVk(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::eNone: return vk::CullModeFlagBits::eNone;
            case CullMode::eFront: return vk::CullModeFlagBits::eFront;
            case CullMode::eBack: return vk::CullModeFlagBits::eBack;
        }
        return vk::CullModeFlagBits::eNone;
    }
    vk::FrontFace ToVk(RHI::FrontFace face)
    {
        return face == FrontFace::eClockwise ? vk::FrontFace::eClockwise
                                             : vk::FrontFace::eCounterClockwise;
    }
    vk::CompareOp ToVk(RHI::CompareOp op)
    {
        switch (op)
        {
            case CompareOp::eNever: return vk::CompareOp::eNever;
            case CompareOp::eLess: return vk::CompareOp::eLess;
            case CompareOp::eEqual: return vk::CompareOp::eEqual;
            case CompareOp::eLessOrEqual: return vk::CompareOp::eLessOrEqual;
            case CompareOp::eGreater: return vk::CompareOp::eGreater;
            case CompareOp::eNotEqual: return vk::CompareOp::eNotEqual;
            case CompareOp::eGreaterOrEqual:
                return vk::CompareOp::eGreaterOrEqual;
            case CompareOp::eAlways: return vk::CompareOp::eAlways;
        }
        return vk::CompareOp::eAlways;
    }
    vk::BlendFactor ToVk(RHI::BlendFactor factor)
    {
        switch (factor)
        {
            case BlendFactor::eZero: return vk::BlendFactor::eZero;
            case BlendFactor::eOne: return vk::BlendFactor::eOne;
            case BlendFactor::eSrcColor: return vk::BlendFactor::eSrcColor;
            case BlendFactor::eOneMinusSrcColor:
                return vk::BlendFactor::eOneMinusSrcColor;
            case BlendFactor::eDstColor: return vk::BlendFactor::eDstColor;
            case BlendFactor::eOneMinusDstColor:
                return vk::BlendFactor::eOneMinusDstColor;
            case BlendFactor::eSrcAlpha: return vk::BlendFactor::eSrcAlpha;
            case BlendFactor::eOneMinusSrcAlpha:
                return vk::BlendFactor::eOneMinusSrcAlpha;
            case BlendFactor::eDstAlpha: return vk::BlendFactor::eDstAlpha;
            case BlendFactor::eOneMinusDstAlpha:
                return vk::BlendFactor::eOneMinusDstAlpha;
        }
        return vk::BlendFactor::eOne;
    }
    vk::BlendOp ToVk(RHI::BlendOp op)
    {
        switch (op)
        {
            case BlendOp::eAdd: return vk::BlendOp::eAdd;
            case BlendOp::eSubtract: return vk::BlendOp::eSubtract;
            case BlendOp::eReverseSubtract:
                return vk::BlendOp::eReverseSubtract;
            case BlendOp::eMin: return vk::BlendOp::eMin;
            case BlendOp::eMax: return vk::BlendOp::eMax;
        }
        return vk::BlendOp::eAdd;
    }
    vk::ColorComponentFlags ToVk(ColorMask mask)
    {
        vk::ColorComponentFlags flags{};
        if (HasAny(mask, ColorMask::eR)) flags |= vk::ColorComponentFlagBits::eR;
        if (HasAny(mask, ColorMask::eG)) flags |= vk::ColorComponentFlagBits::eG;
        if (HasAny(mask, ColorMask::eB)) flags |= vk::ColorComponentFlagBits::eB;
        if (HasAny(mask, ColorMask::eA)) flags |= vk::ColorComponentFlagBits::eA;
        return flags;
    }
    vk::Filter ToVk(RHI::Filter filter)
    {
        return filter == Filter::eNearest ? vk::Filter::eNearest
                                          : vk::Filter::eLinear;
    }
    vk::SamplerMipmapMode ToVkMipmapMode(RHI::Filter filter)
    {
        return filter == Filter::eNearest ? vk::SamplerMipmapMode::eNearest
                                          : vk::SamplerMipmapMode::eLinear;
    }
    vk::SamplerAddressMode ToVk(AddressMode mode)
    {
        switch (mode)
        {
            case AddressMode::eRepeat: return vk::SamplerAddressMode::eRepeat;
            case AddressMode::eMirroredRepeat:
                return vk::SamplerAddressMode::eMirroredRepeat;
            case AddressMode::eClampToEdge:
                return vk::SamplerAddressMode::eClampToEdge;
            case AddressMode::eClampToBorder:
                return vk::SamplerAddressMode::eClampToBorder;
        }
        return vk::SamplerAddressMode::eRepeat;
    }
    vk::AttachmentLoadOp ToVk(LoadOp op)
    {
        switch (op)
        {
            case LoadOp::eLoad: return vk::AttachmentLoadOp::eLoad;
            case LoadOp::eClear: return vk::AttachmentLoadOp::eClear;
            case LoadOp::eDontCare: return vk::AttachmentLoadOp::eDontCare;
        }
        return vk::AttachmentLoadOp::eDontCare;
    }
    vk::AttachmentStoreOp ToVk(StoreOp op)
    {
        return op == StoreOp::eStore ? vk::AttachmentStoreOp::eStore
                                     : vk::AttachmentStoreOp::eDontCare;
    }
    vk::IndexType ToVk(RHI::IndexType type)
    {
        return type == IndexType::eU16 ? vk::IndexType::eUint16
                                       : vk::IndexType::eUint32;
    }

    //------------------------------------------------------------------------
    // Resource states -------------------------------------------------------
    //------------------------------------------------------------------------

    StateInfo GetStateInfo(ResourceState state, bool asSource)
    {
        using Stage  = vk::PipelineStageFlagBits2;
        using Access = vk::AccessFlagBits2;
        using Layout = vk::ImageLayout;

        constexpr auto shaders = Stage::eVertexShader | Stage::eFragmentShader
                               | Stage::eComputeShader;
        constexpr auto depthTests
            = Stage::eEarlyFragmentTests | Stage::eLateFragmentTests;

        switch (state)
        {
            // Nothing to wait for when the old contents are discarded, but a
            // freshly acquired swapchain image is only available after the
            // acquire semaphore, which Submit waits on at eAllCommands; using
            // eAllCommands as the source scope orders the layout transition
            // after that wait.
            case ResourceState::eUndefined:
                return {asSource ? vk::PipelineStageFlags2(Stage::eAllCommands)
                                 : vk::PipelineStageFlags2(Stage::eNone),
                        Access::eNone, Layout::eUndefined};
            case ResourceState::eGeneral:
                return {Stage::eAllCommands,
                        Access::eMemoryRead | Access::eMemoryWrite,
                        Layout::eGeneral};
            case ResourceState::eVertexBuffer:
                return {Stage::eVertexAttributeInput,
                        Access::eVertexAttributeRead, Layout::eUndefined};
            case ResourceState::eIndexBuffer:
                return {Stage::eIndexInput, Access::eIndexRead,
                        Layout::eUndefined};
            case ResourceState::eUniformBuffer:
                return {shaders, Access::eUniformRead, Layout::eUndefined};
            case ResourceState::eIndirectArgument:
                return {Stage::eDrawIndirect, Access::eIndirectCommandRead,
                        Layout::eUndefined};
            case ResourceState::eShaderRead:
                return {shaders,
                        Access::eShaderSampledRead | Access::eShaderStorageRead,
                        Layout::eShaderReadOnlyOptimal};
            case ResourceState::eShaderWrite:
                return {shaders,
                        Access::eShaderStorageRead | Access::eShaderStorageWrite,
                        Layout::eGeneral};
            case ResourceState::eColorAttachment:
                return {Stage::eColorAttachmentOutput,
                        Access::eColorAttachmentRead
                            | Access::eColorAttachmentWrite,
                        Layout::eColorAttachmentOptimal};
            case ResourceState::eDepthStencilWrite:
                return {depthTests,
                        Access::eDepthStencilAttachmentRead
                            | Access::eDepthStencilAttachmentWrite,
                        Layout::eDepthStencilAttachmentOptimal};
            case ResourceState::eDepthStencilRead:
                return {depthTests | Stage::eFragmentShader,
                        Access::eDepthStencilAttachmentRead
                            | Access::eShaderSampledRead,
                        Layout::eDepthStencilReadOnlyOptimal};
            case ResourceState::eTransferSrc:
                return {Stage::eAllTransfer, Access::eTransferRead,
                        Layout::eTransferSrcOptimal};
            case ResourceState::eTransferDst:
                return {Stage::eAllTransfer, Access::eTransferWrite,
                        Layout::eTransferDstOptimal};
            // Presentation reads the image after the semaphore Submit
            // signals; the barrier itself only needs the layout.
            case ResourceState::ePresent:
                return {asSource ? vk::PipelineStageFlags2(Stage::eAllCommands)
                                 : vk::PipelineStageFlags2(Stage::eNone),
                        Access::eNone, Layout::ePresentSrcKHR};
        }
        return {};
    }

    //------------------------------------------------------------------------
    // Debug names -----------------------------------------------------------
    //------------------------------------------------------------------------

    void SetDebugName(vk::Device device, vk::ObjectType type, u64 handle,
                      StringView name)
    {
        if (name.Empty() || !VULKAN_HPP_DEFAULT_DISPATCHER.vkSetDebugUtilsObjectNameEXT)
            return;

        // The name must be NUL-terminated; StringView may not be.
        String                          terminated(name);
        vk::DebugUtilsObjectNameInfoEXT info{};
        info.objectType   = type;
        info.objectHandle = handle;
        info.pObjectName  = terminated.Raw();
        VkCall(device.setDebugUtilsObjectNameEXT(&info));
    }
} // namespace Vortex::Vulkan
