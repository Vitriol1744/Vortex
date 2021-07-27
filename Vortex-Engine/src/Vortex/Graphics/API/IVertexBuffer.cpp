//
// Created by Vitriol1744 on 27.06.2021.
//
#include "IVertexBuffer.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/OpenGL46/GL46VertexBuffer.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IVertexBuffer> IVertexBuffer::Create()
    {
        return Create(0);
    }

    Ref<IVertexBuffer> IVertexBuffer::Create(uint32 size)
    {
        Ref<IVertexBuffer> result;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46VertexBuffer>(size);
                break;

            case GraphicsAPI::None:
            default:
                result = nullptr;
                break;
        }

        return result;
    }

    Ref<IVertexBuffer> IVertexBuffer::Create(void* vertices, uint32 size)
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