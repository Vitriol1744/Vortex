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
            virtual inline uint32 Width() { return width; }
            virtual inline uint32 Height() { return height; }

            virtual void Bind(uint8 slot = 0)   const noexcept = 0;
            virtual void Unbind()               const noexcept = 0;

            //TODO:virtual void SetData(void* data, uint32 size) = 0;

        protected:
            uint32 width;
            uint32 height;
    };
}