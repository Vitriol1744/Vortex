//
// Created by vitriol1744 on 03.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/IIndexBuffer.hpp"
#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46IndexBuffer final : public IIndexBuffer
    {
        public:
            GL46IndexBuffer() noexcept;
            GL46IndexBuffer(GLuint* indices, GLuint size);
            ~GL46IndexBuffer() noexcept override;

            GLvoid Bind() const noexcept override;
            GLvoid Unbind() const noexcept override;

            GLvoid SetData(GLuint* indices, GLuint size) const override;

        private:
            GLuint id = 0;
    };
}
