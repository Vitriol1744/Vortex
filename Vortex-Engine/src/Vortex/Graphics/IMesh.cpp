//
// Created by vitriol1744 on 11.07.2021.
//
#include "IMesh.hpp"

#include "Graphics/IRendererAPI.hpp"
#include "Graphics/OpenGL46/GL46Mesh.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IMesh> IMesh::Create()
    {
        Ref<IMesh> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46Mesh>();
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