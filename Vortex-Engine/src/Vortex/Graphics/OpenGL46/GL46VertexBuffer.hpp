//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/IVertexBuffer.hpp"
#include "Graphics/OpenGL46/OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46VertexBuffer final : public IVertexBuffer
    {
        public:
            GL46VertexBuffer() noexcept = default;
            GL46VertexBuffer(GLfloat* vertices, GLuint size);
            ~GL46VertexBuffer() noexcept override;

            GLvoid Bind() const noexcept override;
            GLvoid Unbind() const noexcept override;

            virtual GLvoid SetData(GLuint* vertices, GLuint size) const override;

        private:
            GLuint id = 0;
    };
}
