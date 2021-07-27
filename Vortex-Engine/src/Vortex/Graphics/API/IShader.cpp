//
// Created by vitriol1744 on 14.07.2021.
//
#include "IShader.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/OpenGL46/GL46Shader.hpp"

#undef None

namespace Vortex::Graphics
{
    Ref<IShader> IShader::Create(std::string_view vertexPath, std::string_view pixelPath, bool precompiled)
    {
        Ref<IShader> result;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::OpenGL46:
                result = CreateRef<GL46Shader>(vertexPath, pixelPath, precompiled);
                break;
            case GraphicsAPI::None:

            default:
                result = nullptr;
                break;
        }

        return result;
    }
}
