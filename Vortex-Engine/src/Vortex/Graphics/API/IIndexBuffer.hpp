//
// Created by Vitriol1744 on 18.09.2022.
//
#ifndef VORTEX_IINDEXBUFFER_HPP
#define VORTEX_IINDEXBUFFER_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API IIndexBuffer
    {
        public:
            virtual ~IIndexBuffer() = default;

            virtual void SetData(void* buffer, uint32 size, uint32 offset = 0) = 0;

            virtual void Bind() const = 0;

            virtual uint32 GetSize() const = 0;

            static Ref<IIndexBuffer> Create(uint16* indices, uint32 size);
    };
}

#endif //VORTEX_IINDEXBUFFER_HPP
