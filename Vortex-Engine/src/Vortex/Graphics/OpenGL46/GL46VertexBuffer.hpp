//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/VertexBuffer.hpp"

namespace Vortex
{
    class VT_API GL46VertexBuffer : public VertexBuffer
    {
        public:
            GL46VertexBuffer(float* vertices, uint32 size);
            ~GL46VertexBuffer() override;

            void Bind() override;
            void Unbind() override;

        private:
            uint32 id;
    };
}
