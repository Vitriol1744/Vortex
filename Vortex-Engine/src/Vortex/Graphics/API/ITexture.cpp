//
// Created by vitriol1744 on 7/26/21.
//
#include "ITexture.hpp"

#include "Vortex/Graphics/API/OpenGL46/GL46Texture.hpp"
#include "Vortex/Graphics/API/IRendererAPI.hpp"

namespace Vortex::Graphics
{
    Ref<ITexture> ITexture::Create()
    {
        Ref<ITexture> result;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46Texture>();
                break;
            case GraphicsAPI::None:

            default:
                result = nullptr;
                break;
        }

        return result;
    }
}
