//
// Created by vitriol1744 on 03.07.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API IIndexBuffer
    {
        public:
            IIndexBuffer() noexcept = default;
            virtual ~IIndexBuffer() noexcept = default;
            
            virtual void Bind() const noexcept = 0;
            virtual void Unbind() const noexcept = 0;

            virtual void Data(uint32* indices, uint32 size) = 0;

            static Ref<IIndexBuffer> Create();
            static Ref<IIndexBuffer> Create(uint32 size);
            static Ref<IIndexBuffer> Create(uint32* indices, uint32 size);
    };
}
