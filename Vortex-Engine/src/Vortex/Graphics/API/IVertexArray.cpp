//
// Created by vitriol1744 on 11.07.2021.
//
#include "IVertexArray.hpp"

#include "IRendererAPI.hpp"
#include "Graphics/API/OpenGL46/GL46VertexArray.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IVertexArray> IVertexArray::Create()
    {
        Ref<IVertexArray> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46VertexArray>();
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