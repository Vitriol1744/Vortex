//
// Created by Vitriol1744 on 18.09.2022.
//
#ifndef VORTEX_IUNIFORMBUFFER_HPP
#define VORTEX_IUNIFORMBUFFER_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API IUniformBuffer
    {
        public:
            virtual ~IUniformBuffer() = default;

            virtual void SetData(void* data) = 0;
    };
}

#endif //VORTEX_IUNIFORMBUFFER_HPP
