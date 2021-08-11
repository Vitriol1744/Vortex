//
// Created by vitriol1744 on 7/26/21.
//
#include "ITexture2D.hpp"

#include "Vortex/Graphics/API/OpenGL46/GL46Texture2D.hpp"
#include "Vortex/Graphics/API/IRendererAPI.hpp"

namespace Vortex::Graphics
{
    Ref<ITexture2D> ITexture2D::Create(strview filepath)
    {
        Ref<ITexture2D> result;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46Texture2D>(filepath);
                break;
            case GraphicsAPI::None:

            default:
                result = nullptr;
                break;
        }

        return result;
    }
}
