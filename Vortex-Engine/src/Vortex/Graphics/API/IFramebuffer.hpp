//
// Created by Vitriol1744 on 11.09.2022.
//
#ifndef VORTEX_IFRAMEBUFFER_HPP
#define VORTEX_IFRAMEBUFFER_HPP

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IGraphicsPipeline.hpp"

namespace Vortex::Graphics
{
    class VT_API IFramebuffer
    {
        public:
            virtual ~IFramebuffer() = default;
            virtual void Bind() = 0;

            static Ref<IFramebuffer> Create(Ref<IGraphicsPipeline> pipeline);
    };
}

#endif //VORTEX_IFRAMEBUFFER_HPP
