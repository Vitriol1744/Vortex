//
// Created by vitriol1744 on 7/26/21.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/ITexture.hpp"

namespace Vortex::Graphics
{
    class VT_API ITexture2D : public ITexture
    {
        public:
            static Ref<ITexture2D> Create(strview filepath);
    };
}