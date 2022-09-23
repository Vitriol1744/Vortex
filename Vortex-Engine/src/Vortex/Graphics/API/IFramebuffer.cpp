//
// Created by Vitriol1744 on 11.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IFramebuffer.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanFramebuffer.hpp"

namespace Vortex::Graphics
{
    Ref<IFramebuffer> IFramebuffer::Create(Ref<IGraphicsPipeline> pipeline)
    {
        Ref<IFramebuffer> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::eVulkan:
                result = CreateRef<VulkanFramebuffer>(pipeline);
                break;

            case GraphicsAPI::eNone:
            default:
                break;
        }

        VT_CORE_ASSERT(result != nullptr);
        return result;
    }
}