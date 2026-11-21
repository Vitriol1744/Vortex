/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

export module Vortex.RHI.Pipeline;

export import Vortex.RHI.Shader;

import Prism.Containers.Vector;
import Prism.Memory.Ref;
import Prism.Memory.RefCounted;
import Prism.String.String;

/**
 * Pipelines bake a shader with all fixed-function state. They are created
 * against attachment *formats*, not a framebuffer, so one pipeline renders
 * into any target with matching formats (dynamic rendering).
 */
export namespace Vortex::RHI
{
    struct VertexAttribute
    {
        /// `layout(location = N)` of the shader input.
        u32         Location = 0;
        RHI::Format Format   = RHI::Format::eUndefined;
        /// Byte offset inside one vertex.
        u32         Offset   = 0;
    };

    /// One vertex buffer slot: stride and the attributes read from it.
    struct VertexBinding
    {
        u32                     Stride      = 0;
        bool                    PerInstance = false;
        Vector<VertexAttribute> Attributes;

        /// Builds a tightly packed binding: locations 0..N-1 in order.
        static inline VertexBinding Packed(InitializerList<RHI::Format> formats,
                                           bool perInstance = false)
        {
            VertexBinding binding;
            binding.PerInstance = perInstance;
            u32 location        = 0;
            for (RHI::Format format : formats)
            {
                binding.Attributes.PushBack({location++, format, binding.Stride});
                binding.Stride += GetFormatInfo(format).BytesPerBlock;
            }
            return binding;
        }
    };

    struct GraphicsPipelineDesc
    {
        ::Ref<RHI::Shader>      Shader = nullptr;
        /// Slot i of this vector is vertex buffer binding i.
        Vector<VertexBinding>   VertexBindings;
        RHI::Topology           Topology = RHI::Topology::eTriangleList;
        RasterState             Raster;
        DepthStencilState       DepthStencil;
        /// One per color attachment; missing entries mean "no blending".
        Vector<BlendAttachment> Blend;
        Vector<RHI::Format>     ColorFormats;
        RHI::Format             DepthFormat = RHI::Format::eUndefined;
        u32                     Samples     = 1;
        String                  DebugName{};
    };

    struct ComputePipelineDesc
    {
        /// A shader with a single compute stage.
        ::Ref<RHI::Shader> Shader = nullptr;
        String             DebugName{};
    };

    enum class PipelineType : u8
    {
        eGraphics,
        eCompute,
    };

    class VT_RENDERER_RHI_API Pipeline : public RefCounted<Pipeline>
    {
      public:
        Pipeline(const Pipeline&)            = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        virtual ~Pipeline()                  = default;

        inline PipelineType Type() const { return m_Type; }
        /// Fixed-function state of a graphics pipeline (empty for compute).
        inline const GraphicsPipelineDesc& Desc() const { return m_Desc; }
        inline ::Ref<RHI::Shader>          Shader() const { return m_Shader; }

      protected:
        explicit Pipeline(const GraphicsPipelineDesc& desc)
            : m_Type(PipelineType::eGraphics)
            , m_Desc(desc)
            , m_Shader(desc.Shader)
        {
        }
        explicit Pipeline(const ComputePipelineDesc& desc)
            : m_Type(PipelineType::eCompute)
            , m_Shader(desc.Shader)
        {
            m_Desc.DebugName = desc.DebugName;
        }

        PipelineType         m_Type;
        GraphicsPipelineDesc m_Desc;
        ::Ref<RHI::Shader>   m_Shader;
    };
} // namespace Vortex::RHI
