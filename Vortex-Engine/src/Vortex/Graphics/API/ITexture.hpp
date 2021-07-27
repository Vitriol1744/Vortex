//
// Created by vitriol1744 on 7/26/21.
//
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    class VT_API ITexture
    {
        public:
            virtual void Bind() const noexcept = 0;
            virtual void Unbind() const noexcept = 0;

            static Ref<ITexture> Create();
    };
}