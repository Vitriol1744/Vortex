//
// Created by vitriol1744 on 11.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "IVertexBuffer.hpp"
#include "IIndexBuffer.hpp"

namespace Vortex::Graphics
{
    class VT_API IVertexArray
    {
        public:
            virtual ~IVertexArray() = default;

            virtual void Bind() = 0;
            virtual void Unbind() = 0;

            virtual void AddVertexBuffer(Ref<IVertexBuffer> vertexBuffer) = 0;
            virtual void SetIndexBuffer(Ref<IIndexBuffer> indexBuffer) = 0;

            static Ref<IVertexArray> Create();
    };
}