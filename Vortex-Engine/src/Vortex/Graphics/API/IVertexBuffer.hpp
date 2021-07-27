//
// Created by Vitriol1744 on 27.06.2021.
//
#pragma once

#include <utility>

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/VertexBufferLayout.hpp"

namespace Vortex::Graphics
{
    class VT_API IVertexBuffer
    {
        public:
            virtual ~IVertexBuffer() = default;

            VT_NODISCARD inline VertexBufferLayout GetLayout() const { return layout; }
            inline void SetLayout(VertexBufferLayout layout) { this->layout = std::move(layout); }

            virtual void Bind() const noexcept = 0;
            virtual void Unbind() const noexcept = 0;

            virtual void SetData(void* vertices, uint32 size) const = 0;

            static Ref<IVertexBuffer> Create();
            static Ref<IVertexBuffer> Create(uint32 size);
            static Ref<IVertexBuffer> Create(void* vertices, uint32 size);

        private:
            VertexBufferLayout layout;
    };
}
