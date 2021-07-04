//
// Created by vitriol1744 on 03.07.2021.
//
#include "IndexBuffer.hpp"

#include "Graphics/IRendererAPI.hpp"
#include "Graphics/OpenGL46/GL46IndexBuffer.hpp"

namespace Vortex
{
    Ref<IndexBuffer> IndexBuffer::Create(uint32* indices, uint32 size)
    {
        Ref<IndexBuffer> result = nullptr;
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