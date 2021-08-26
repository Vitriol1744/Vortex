//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IVertexBuffer.hpp"
#include "Vortex/Platform/OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46VertexBuffer final : public IVertexBuffer
    {
        public:
            GL46VertexBuffer() noexcept;
            GL46VertexBuffer(GLuint size);
            GL46VertexBuffer(GLvoid* vertices, GLuint size);
            GL46VertexBuffer(const GL46VertexBuffer& vertexBuffer);
            ~GL46VertexBuffer() noexcept override;

            GLvoid Bind() const noexcept override;
            GLvoid Unbind() const noexcept override;

            virtual GLvoid SetData(GLvoid* vertices, GLuint size) const override;

        private:
            GLuint id = 0;
            GLsizei size;

            inline GLuint GetID() const noexcept { return id; }
            inline GLsizei GetSize() const noexcept { return size; }
    };
}
