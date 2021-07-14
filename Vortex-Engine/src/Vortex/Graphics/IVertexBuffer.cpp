//
// Created by Vitriol1744 on 27.06.2021.
//
#include "IVertexBuffer.hpp"

#include "Graphics/IRendererAPI.hpp"
#include "Graphics/OpenGL46/GL46VertexBuffer.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IVertexBuffer> IVertexBuffer::Create(float* vertices, uint32 size)
    {
        Ref<IVertexBuffer> result;
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