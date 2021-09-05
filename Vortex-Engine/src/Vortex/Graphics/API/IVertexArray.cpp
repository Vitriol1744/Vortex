//
// Created by vitriol1744 on 11.07.2021.
//
#include "IVertexArray.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/OpenGL46/GL46VertexArray.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IVertexArray> IVertexArray::Create()
    {
        Ref<IVertexArray> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::eOpenGL46:
                result = CreateRef<GL46VertexArray>();
                break;
            case GraphicsAPI::eVulkan:
                //TODO: Vulkan: IVertexArray Impl
                result = nullptr;
                return result;
                break;

            case GraphicsAPI::eNone:
            default:
                result = nullptr;
                break;
        }

        VT_CORE_ASSERT(result != nullptr);
        return result;
    }
}