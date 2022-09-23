//
// Created by Vitriol1744 on 08.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IRendererAPI.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"

namespace Vortex::Graphics
{
    GraphicsAPI IRendererAPI::api = GraphicsAPI::eNone;
    IRendererAPI* IRendererAPI::instance = nullptr;

    void IRendererAPI::Initialize(GraphicsAPI api, std::string_view applicationName, Math::Vec3 applicationVersion)
    {
        IRendererAPI::api = api;
        switch (api)
        {
            case GraphicsAPI::eVulkan:
                instance = new VulkanRendererAPI;
                break;

            case GraphicsAPI::eNone:
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