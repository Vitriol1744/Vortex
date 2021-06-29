//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex
{
    class VT_API VertexBuffer
    {
        public:
            virtual ~VertexBuffer() = default;

            virtual void Bind() = 0;
            virtual void Unbind() = 0;

            static Ref<VertexBuffer> Create(float* vertices, uint32 size);
    };
}
