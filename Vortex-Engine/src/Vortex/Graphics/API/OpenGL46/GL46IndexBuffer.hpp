//
// Created by vitriol1744 on 03.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/API/IIndexBuffer.hpp"
#include "OpenGL.hpp"

namespace Vortex::Graphics
{
    class VT_API GL46IndexBuffer final : public IIndexBuffer
    {
        public:
            GL46IndexBuffer() noexcept;
            GL46IndexBuffer(GLuint size) noexcept;
            GL46IndexBuffer(GLuint* indices, GLuint size);
            virtual ~GL46IndexBuffer() noexcept override;

            virtual GLvoid Bind() const noexcept override;
            virtual GLvoid Unbind() const noexcept override;

            virtual inline GLuint GetIndicesCount() const noexcept override { return count; }

            virtual GLvoid SetData(GLuint* indices, GLuint size) override;

        private:
            GLuint id = 0;
            GLuint count = 0;
    };
}
