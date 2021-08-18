//
// Created by vitriol1744 on 11.07.2021.
//
#pragma once

#include "Core/Core.hpp"

#include "Graphics/API/IVertexArray.hpp"
#include "OpenGL.hpp"

#include <vector>

namespace Vortex::Graphics
{
    class VT_API GL46VertexArray final : public IVertexArray
    {
        public:
            GL46VertexArray() noexcept;
            ~GL46VertexArray() noexcept;

            GLvoid Bind() override;
            GLvoid Unbind() noexcept override;

            virtual inline std::vector<Ref<IVertexBuffer>>& GetVertexBuffers() noexcept override { return vertexBuffers; }
            virtual inline Ref<IIndexBuffer>& GetIndexBuffer() noexcept override { return indexBuffer; }

            GLvoid AddVertexBuffer(const Ref<IVertexBuffer>& vertexBuffer) override;
            GLvoid SetIndexBuffer(const Ref<IIndexBuffer>& indexBuffer) override;

        private:
            GLuint id = 0;

            std::vector<Ref<IVertexBuffer>> vertexBuffers;
            Ref<IIndexBuffer> indexBuffer;
    };
}