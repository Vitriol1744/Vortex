/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>

export module Vortex.RHI.Types;

import Prism.Core.TypeTraits;

/**
 * Backend-agnostic vocabulary of the RHI: formats, usages, resource states and
 * the fixed-function state descriptions pipelines are built from. Nothing here
 * refers to Vulkan (or any other API); backends translate these values.
 */
export namespace Vortex::RHI
{
    //------------------------------------------------------------------------
    // Flags -----------------------------------------------------------------
    //------------------------------------------------------------------------

    /// Opt an enum into the bitwise operators below by specializing this.
    template <typename E>
    inline constexpr bool EnableFlags = false;

    template <typename E>
    concept FlagEnum = EnableFlags<E>;

    template <FlagEnum E>
    constexpr E operator|(E lhs, E rhs)
    {
        using U = Prism::UnderlyingTypeType<E>;
        return static_cast<E>(static_cast<U>(lhs) | static_cast<U>(rhs));
    }
    template <FlagEnum E>
    constexpr E operator&(E lhs, E rhs)
    {
        using U = Prism::UnderlyingTypeType<E>;
        return static_cast<E>(static_cast<U>(lhs) & static_cast<U>(rhs));
    }
    template <FlagEnum E>
    constexpr E operator~(E value)
    {
        using U = Prism::UnderlyingTypeType<E>;
        return static_cast<E>(~static_cast<U>(value));
    }
    template <FlagEnum E>
    constexpr E& operator|=(E& lhs, E rhs)
    {
        return lhs = lhs | rhs;
    }
    template <FlagEnum E>
    constexpr E& operator&=(E& lhs, E rhs)
    {
        return lhs = lhs & rhs;
    }
    /// True if any of `bits` is set in `value`.
    template <FlagEnum E>
    constexpr bool HasAny(E value, E bits)
    {
        return static_cast<Prism::UnderlyingTypeType<E>>(value & bits) != 0;
    }
    /// True if all of `bits` are set in `value`.
    template <FlagEnum E>
    constexpr bool HasAll(E value, E bits)
    {
        return (value & bits) == bits;
    }

    //------------------------------------------------------------------------
    // Geometry --------------------------------------------------------------
    //------------------------------------------------------------------------

    struct Extent2D
    {
        u32                  Width  = 0;
        u32                  Height = 0;
        constexpr bool       operator==(const Extent2D&) const = default;
        constexpr bool       Empty() const { return Width == 0 || Height == 0; }
    };
    struct Extent3D
    {
        u32            Width  = 0;
        u32            Height = 0;
        u32            Depth  = 1;
        constexpr bool operator==(const Extent3D&) const = default;
    };
    struct Offset2D
    {
        i32            X = 0;
        i32            Y = 0;
        constexpr bool operator==(const Offset2D&) const = default;
    };
    struct Rect2D
    {
        Offset2D       Offset{};
        Extent2D       Extent{};
        constexpr bool operator==(const Rect2D&) const = default;
    };
    struct Viewport
    {
        f32 X        = 0.0f;
        f32 Y        = 0.0f;
        f32 Width    = 0.0f;
        f32 Height   = 0.0f;
        f32 MinDepth = 0.0f;
        f32 MaxDepth = 1.0f;
    };

    //------------------------------------------------------------------------
    // Formats ---------------------------------------------------------------
    //------------------------------------------------------------------------

    /// Texel and vertex-attribute formats. Names list components in memory
    /// order, then the numeric type.
    enum class Format : u16
    {
        eUndefined,

        // 8 bit per component
        eR8Unorm,
        eR8Uint,
        eRG8Unorm,
        eRGBA8Unorm,
        eRGBA8Srgb,
        eRGBA8Uint,
        eBGRA8Unorm,
        eBGRA8Srgb,

        // 16 bit per component
        eR16Uint,
        eR16Float,
        eRG16Float,
        eRGBA16Float,

        // 32 bit per component
        eR32Uint,
        eR32Sint,
        eR32Float,
        eRG32Uint,
        eRG32Sint,
        eRG32Float,
        eRGB32Uint,
        eRGB32Sint,
        eRGB32Float,
        eRGBA32Uint,
        eRGBA32Sint,
        eRGBA32Float,

        // Packed
        eRGB10A2Unorm,
        eRG11B10Float,

        // Depth / stencil
        eD16Unorm,
        eD32Float,
        eD24UnormS8Uint,
        eD32FloatS8Uint,

        // Block compressed
        eBC1RGBAUnorm,
        eBC1RGBASrgb,
        eBC3RGBAUnorm,
        eBC3RGBASrgb,
        eBC4RUnorm,
        eBC5RGUnorm,
        eBC6HRGBFloat,
        eBC7RGBAUnorm,
        eBC7RGBASrgb,

        eCount,
    };

    struct FormatInfo
    {
        /// Bytes per texel, or per block for compressed formats.
        u8   BytesPerBlock = 0;
        /// Width and height of a block (1 for uncompressed formats).
        u8   BlockSize     = 1;
        u8   Components    = 0;
        bool Depth         = false;
        bool Stencil       = false;
        bool Srgb          = false;
        bool Compressed    = false;
        bool Integer       = false;
    };

    constexpr FormatInfo GetFormatInfo(Format format)
    {
        switch (format)
        {
            case Format::eUndefined:
            case Format::eCount: return {};

            case Format::eR8Unorm: return {1, 1, 1};
            case Format::eR8Uint: return {1, 1, 1, false, false, false, false, true};
            case Format::eRG8Unorm: return {2, 1, 2};
            case Format::eRGBA8Unorm: return {4, 1, 4};
            case Format::eRGBA8Srgb: return {4, 1, 4, false, false, true};
            case Format::eRGBA8Uint: return {4, 1, 4, false, false, false, false, true};
            case Format::eBGRA8Unorm: return {4, 1, 4};
            case Format::eBGRA8Srgb: return {4, 1, 4, false, false, true};

            case Format::eR16Uint: return {2, 1, 1, false, false, false, false, true};
            case Format::eR16Float: return {2, 1, 1};
            case Format::eRG16Float: return {4, 1, 2};
            case Format::eRGBA16Float: return {8, 1, 4};

            case Format::eR32Uint:
            case Format::eR32Sint: return {4, 1, 1, false, false, false, false, true};
            case Format::eR32Float: return {4, 1, 1};
            case Format::eRG32Uint:
            case Format::eRG32Sint: return {8, 1, 2, false, false, false, false, true};
            case Format::eRG32Float: return {8, 1, 2};
            case Format::eRGB32Uint:
            case Format::eRGB32Sint: return {12, 1, 3, false, false, false, false, true};
            case Format::eRGB32Float: return {12, 1, 3};
            case Format::eRGBA32Uint:
            case Format::eRGBA32Sint: return {16, 1, 4, false, false, false, false, true};
            case Format::eRGBA32Float: return {16, 1, 4};

            case Format::eRGB10A2Unorm: return {4, 1, 4};
            case Format::eRG11B10Float: return {4, 1, 3};

            case Format::eD16Unorm: return {2, 1, 1, true};
            case Format::eD32Float: return {4, 1, 1, true};
            case Format::eD24UnormS8Uint: return {4, 1, 2, true, true};
            case Format::eD32FloatS8Uint: return {8, 1, 2, true, true};

            case Format::eBC1RGBAUnorm: return {8, 4, 4, false, false, false, true};
            case Format::eBC1RGBASrgb: return {8, 4, 4, false, false, true, true};
            case Format::eBC3RGBAUnorm: return {16, 4, 4, false, false, false, true};
            case Format::eBC3RGBASrgb: return {16, 4, 4, false, false, true, true};
            case Format::eBC4RUnorm: return {8, 4, 1, false, false, false, true};
            case Format::eBC5RGUnorm: return {16, 4, 2, false, false, false, true};
            case Format::eBC6HRGBFloat: return {16, 4, 3, false, false, false, true};
            case Format::eBC7RGBAUnorm: return {16, 4, 4, false, false, false, true};
            case Format::eBC7RGBASrgb: return {16, 4, 4, false, false, true, true};
        }
        return {};
    }
    constexpr bool IsDepthFormat(Format format)
    {
        return GetFormatInfo(format).Depth;
    }
    constexpr bool HasStencil(Format format)
    {
        return GetFormatInfo(format).Stencil;
    }
    /// Bytes needed for one mip level of `extent` texels in `format`.
    constexpr usize FormatSize(Format format, Extent3D extent)
    {
        FormatInfo info   = GetFormatInfo(format);
        usize      blockW = (extent.Width + info.BlockSize - 1) / info.BlockSize;
        usize      blockH = (extent.Height + info.BlockSize - 1) / info.BlockSize;
        return blockW * blockH * extent.Depth * info.BytesPerBlock;
    }

    //------------------------------------------------------------------------
    // Usages ----------------------------------------------------------------
    //------------------------------------------------------------------------

    enum class BufferUsage : u32
    {
        eNone        = 0,
        eVertex      = 1 << 0,
        eIndex       = 1 << 1,
        eUniform     = 1 << 2,
        eStorage     = 1 << 3,
        eIndirect    = 1 << 4,
        eTransferSrc = 1 << 5,
        eTransferDst = 1 << 6,
    };
    template <>
    inline constexpr bool EnableFlags<BufferUsage> = true;

    enum class TextureUsage : u32
    {
        eNone                   = 0,
        eSampled                = 1 << 0,
        eStorage                = 1 << 1,
        eColorAttachment        = 1 << 2,
        eDepthStencilAttachment = 1 << 3,
        eTransferSrc            = 1 << 4,
        eTransferDst            = 1 << 5,
    };
    template <>
    inline constexpr bool EnableFlags<TextureUsage> = true;

    /// Where a resource's memory lives and who can access it.
    enum class MemoryUsage : u8
    {
        /// Device-local, not mappable. Write it through uploads.
        eGpuOnly,
        /// Host-visible, written by the CPU every frame (uniforms, dynamic
        /// geometry, staging).
        eCpuToGpu,
        /// Host-visible and cached, read back by the CPU.
        eGpuToCpu,
    };

    enum class TextureType : u8
    {
        e2D,
        e2DArray,
        eCube,
        e3D,
    };

    enum class QueueType : u8
    {
        eGraphics,
        eCompute,
        eTransfer,
    };

    enum class ShaderStage : u32
    {
        eNone         = 0,
        eVertex       = 1 << 0,
        eFragment     = 1 << 1,
        eCompute      = 1 << 2,
        eGeometry     = 1 << 3,
        eTessControl  = 1 << 4,
        eTessEval     = 1 << 5,
        eAllGraphics  = eVertex | eFragment | eGeometry | eTessControl
                     | eTessEval,
        eAll          = eAllGraphics | eCompute,
    };
    template <>
    inline constexpr bool EnableFlags<ShaderStage> = true;

    //------------------------------------------------------------------------
    // Resource states -------------------------------------------------------
    //------------------------------------------------------------------------

    /// How a resource is about to be used. Barriers transition between these;
    /// backends derive stages, access masks and image layouts from them.
    enum class ResourceState : u8
    {
        eUndefined,
        eGeneral,
        eVertexBuffer,
        eIndexBuffer,
        eUniformBuffer,
        eIndirectArgument,
        /// Sampled or read as a texel/storage buffer by any shader stage.
        eShaderRead,
        /// Written (and possibly read) as storage by any shader stage.
        eShaderWrite,
        eColorAttachment,
        eDepthStencilWrite,
        eDepthStencilRead,
        eTransferSrc,
        eTransferDst,
        ePresent,
    };

    //------------------------------------------------------------------------
    // Fixed-function state --------------------------------------------------
    //------------------------------------------------------------------------

    enum class LoadOp : u8
    {
        eLoad,
        eClear,
        eDontCare,
    };
    enum class StoreOp : u8
    {
        eStore,
        eDontCare,
    };

    enum class IndexType : u8
    {
        eU16,
        eU32,
    };

    enum class Topology : u8
    {
        eTriangleList,
        eTriangleStrip,
        eLineList,
        eLineStrip,
        ePointList,
    };
    enum class PolygonMode : u8
    {
        eFill,
        eLine,
        ePoint,
    };
    enum class CullMode : u8
    {
        eNone,
        eFront,
        eBack,
    };
    enum class FrontFace : u8
    {
        eCounterClockwise,
        eClockwise,
    };
    enum class CompareOp : u8
    {
        eNever,
        eLess,
        eEqual,
        eLessOrEqual,
        eGreater,
        eNotEqual,
        eGreaterOrEqual,
        eAlways,
    };
    enum class BlendFactor : u8
    {
        eZero,
        eOne,
        eSrcColor,
        eOneMinusSrcColor,
        eDstColor,
        eOneMinusDstColor,
        eSrcAlpha,
        eOneMinusSrcAlpha,
        eDstAlpha,
        eOneMinusDstAlpha,
    };
    enum class BlendOp : u8
    {
        eAdd,
        eSubtract,
        eReverseSubtract,
        eMin,
        eMax,
    };
    enum class ColorMask : u8
    {
        eNone = 0,
        eR    = 1 << 0,
        eG    = 1 << 1,
        eB    = 1 << 2,
        eA    = 1 << 3,
        eAll  = eR | eG | eB | eA,
    };
    template <>
    inline constexpr bool EnableFlags<ColorMask> = true;

    struct RasterState
    {
        PolygonMode Polygon   = PolygonMode::eFill;
        CullMode    Cull      = CullMode::eBack;
        FrontFace   Front     = FrontFace::eCounterClockwise;
        bool        DepthClamp = false;
        f32         LineWidth = 1.0f;
    };

    struct DepthStencilState
    {
        bool      DepthTest  = true;
        bool      DepthWrite = true;
        CompareOp DepthCompare = CompareOp::eLess;
        // Stencil tests aren't exposed yet.
    };

    struct BlendAttachment
    {
        bool        Enable    = false;
        BlendFactor SrcColor  = BlendFactor::eOne;
        BlendFactor DstColor  = BlendFactor::eZero;
        BlendOp     ColorOp   = BlendOp::eAdd;
        BlendFactor SrcAlpha  = BlendFactor::eOne;
        BlendFactor DstAlpha  = BlendFactor::eZero;
        BlendOp     AlphaOp   = BlendOp::eAdd;
        ColorMask   WriteMask = ColorMask::eAll;

        /// Standard "over" blending for straight (non-premultiplied) alpha.
        static constexpr BlendAttachment AlphaBlend()
        {
            return {true,
                    BlendFactor::eSrcAlpha,
                    BlendFactor::eOneMinusSrcAlpha,
                    BlendOp::eAdd,
                    BlendFactor::eOne,
                    BlendFactor::eOneMinusSrcAlpha,
                    BlendOp::eAdd,
                    ColorMask::eAll};
        }
    };

    //------------------------------------------------------------------------
    // Samplers --------------------------------------------------------------
    //------------------------------------------------------------------------

    enum class Filter : u8
    {
        eNearest,
        eLinear,
    };
    enum class AddressMode : u8
    {
        eRepeat,
        eMirroredRepeat,
        eClampToEdge,
        eClampToBorder,
    };

    //------------------------------------------------------------------------
    // Clear values ----------------------------------------------------------
    //------------------------------------------------------------------------

    struct ClearColor
    {
        f32 R = 0.0f;
        f32 G = 0.0f;
        f32 B = 0.0f;
        f32 A = 1.0f;
    };
    struct ClearDepthStencil
    {
        f32 Depth   = 1.0f;
        u32 Stencil = 0;
    };

    /// Limits a device guarantees; query them instead of hard-coding.
    struct DeviceLimits
    {
        u32   MaxTextureDimension2D       = 0;
        u32   MaxColorAttachments         = 0;
        u32   MaxPushConstantSize         = 0;
        u32   MaxBoundBindGroups          = 0;
        usize MinUniformBufferAlignment   = 0;
        usize MinStorageBufferAlignment   = 0;
        f32   MaxSamplerAnisotropy        = 0.0f;
        f32   TimestampPeriodNanoseconds  = 0.0f;
    };
    struct DeviceFeatures
    {
        bool SamplerAnisotropy    = false;
        bool TextureCompressionBC = false;
        bool FillModeNonSolid     = false;
        bool TimestampQueries     = false;
    };

    struct MemoryStats
    {
        usize DeviceLocalUsage  = 0;
        usize DeviceLocalBudget = 0;
    };
} // namespace Vortex::RHI
