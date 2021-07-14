//
// Created by vitriol1744 on 11.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/IVertexBuffer.hpp"
#include "Graphics/IIndexBuffer.hpp"

namespace Vortex::Graphics
{
    class VT_API IMesh
    {
        public:
            virtual ~IMesh() = default;

            virtual void Bind() = 0;
            virtual void Unbind() = 0;

            virtual void AddVertexBuffer(Ref<IVertexBuffer> vertexBuffer) = 0;
            virtual void SetIndexBuffer(Ref<IIndexBuffer> indexBuffer) = 0;

            static Ref<IMesh> Create();
    };
}