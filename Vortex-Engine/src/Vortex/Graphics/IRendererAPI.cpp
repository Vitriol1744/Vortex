//
// Created by Vitriol1744 on 27.06.2021.
//
#include "IRendererAPI.hpp"

#include "Graphics/OpenGL46/GL46RendererAPI.hpp"

namespace Vortex
{
    GraphicsAPI IRendererAPI::api = GraphicsAPI::None;

    Scope<IRendererAPI>& IRendererAPI::Get()
    {
        static Scope<IRendererAPI> instance = CreateRendererAPI();

        return instance;
    }

    void IRendererAPI::Initialize()
    {
        api = GraphicsAPI::OpenGL46;
        Get(); // Create Instance of RendererAPI
    }

    Scope<IRendererAPI> IRendererAPI::CreateRendererAPI()
    {
        Scope<IRendererAPI> result;

        switch (api)
        {
            case GraphicsAPI::OpenGL46:
                result = CreateScope<GL46RendererAPI>();
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