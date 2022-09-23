//
// Created by Vitriol1744 on 16.09.2022.
//
#ifndef VORTEX_IVERTEXBUFFER_HPP
#define VORTEX_IVERTEXBUFFER_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    enum class VertexBufferUsage
    {
        eStatic,
        eDynamic
    };
    class VT_API IVertexBuffer
    {
        public:
            virtual ~IVertexBuffer() = default;

            virtual void SetData(void* buffer, uint32 size, uint32 offset = 0) = 0;

            virtual void Bind() const = 0;

            virtual uint32 GetSize() const = 0;

            static Ref<IVertexBuffer> Create(void* vertices, uint32 size);
    };
}

#endif //VORTEX_IVERTEXBUFFER_HPP
