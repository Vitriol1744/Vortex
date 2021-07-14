//
// Created by vitriol1744 on 03.07.2021.
//
#pragma once

#include "Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API IIndexBuffer
    {
        public:
            IIndexBuffer() noexcept = default;
            virtual ~IIndexBuffer() noexcept = default;
            
            virtual void Bind() const noexcept = 0;
            virtual void Unbind() const noexcept = 0;

            virtual void SetData(uint32* indices, uint32 size) const = 0;
            
            static Ref<IIndexBuffer> Create(uint32* indices, uint32 size);
    };
}
