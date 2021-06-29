//
// Created by Vitriol1744 on 27.06.2021.
//

#include "VertexBuffer.hpp"

#include "Graphics/IRendererAPI.hpp"
#include "Graphics/OpenGL46/GL46VertexBuffer.hpp"

namespace Vortex
{
    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32 size)
    {
        Ref<VertexBuffer> result;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46VertexBuffer>(vertices, size);
                break;

            case GraphicsAPI::None:
            default:
                result = nullptr;
                break;
        }

        return result;
    }
}