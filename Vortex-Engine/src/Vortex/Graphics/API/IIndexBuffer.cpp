//
// Created by vitriol1744 on 03.07.2021.
//
#include "IIndexBuffer.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/OpenGL46/GL46IndexBuffer.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IIndexBuffer> IIndexBuffer::Create()
    {
        return Create(0);
    }

    Ref<IIndexBuffer> IIndexBuffer::Create(uint32 size)
    {
        Ref<IIndexBuffer> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46IndexBuffer>(size);
                break;

            case GraphicsAPI::None:
            default:
                result = nullptr;
                break;
        }

        VT_CORE_ASSERT(result != nullptr);
        return result;
    }

    Ref<IIndexBuffer> IIndexBuffer::Create(uint32* indices, uint32 size)
    {
        Ref<IIndexBuffer> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46IndexBuffer>(indices, size);
                break;
            
            case GraphicsAPI::None:
            default:
                result = nullptr;
                break;
        }
        
        VT_CORE_ASSERT(result != nullptr);
        return result;
    }
}