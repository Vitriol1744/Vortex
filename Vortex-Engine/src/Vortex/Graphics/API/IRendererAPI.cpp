//
// Created by Vitriol1744 on 27.06.2021.
//
#include "vtpch.hpp"
#include "IRendererAPI.hpp"

#include "Vortex/Graphics/API/OpenGL46/GL46RendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

#undef None

namespace Vortex::Graphics
{
    GraphicsAPI IRendererAPI::api = GraphicsAPI::None;
    IRendererAPI* IRendererAPI::instance = nullptr;

    void IRendererAPI::Initialize(GraphicsAPI api, std::string_view applicationName, Math::Vec3 applicationVersion)
    {
        IRendererAPI::api = api;
        switch (api)
        {
            case GraphicsAPI::OpenGL46:
                instance = new GL46RendererAPI;
                break;
            case GraphicsAPI::Vulkan:
                instance = new VkRendererAPI;
                break;

            case GraphicsAPI::None:
            default:
                VTCoreLogError("{}: Unknown API!", __FUNCSIG__);
                instance = nullptr;
                break;
        }

        VT_CORE_ASSERT_MSG(instance, "Failed to Create RendererAPI!");
        instance->applicationName = applicationName;
        instance->applicationVersion = applicationVersion;
        instance->InitializeImpl();
    }
    void IRendererAPI::Shutdown()
    {
        if (instance) delete instance;
        instance = nullptr;
    }
}