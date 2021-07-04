//
// Created by vitriol1744 on 03.07.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex
{
    class IndexBuffer
    {
        public:
            IndexBuffer()
            { };
            virtual ~IndexBuffer() = default;
            
            virtual void Bind() const = 0;
            virtual void Unbind() const = 0;
            
            static Ref<IndexBuffer> Create(uint32* indices, uint32 size);
    };
}
