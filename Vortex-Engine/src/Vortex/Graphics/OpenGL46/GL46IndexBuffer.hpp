//
// Created by vitriol1744 on 03.07.2021.
//
#pragma once

#include "Graphics/IndexBuffer.hpp"

namespace Vortex
{
    class GL46IndexBuffer : public IndexBuffer
    {
        public:
            GL46IndexBuffer(uint32* indices, uint32 size);
            ~GL46IndexBuffer() noexcept override;
            
            void Bind() const override;
            void Unbind() const override;
            
        private:
            uint32 id{};
    };
}
