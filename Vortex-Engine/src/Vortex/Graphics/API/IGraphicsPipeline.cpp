//
// Created by Vitriol1744 on 10.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IGraphicsPipeline.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanGraphicsPipeline.hpp"

namespace Vortex::Graphics
{
    Ref<IGraphicsPipeline> IGraphicsPipeline::Create(GraphicsPipelineCreateInfo& createInfo, Ref<IWindow> window)
    {
        Ref<IGraphicsPipeline> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::eVulkan:
                result = CreateRef<VulkanGraphicsPipeline>(createInfo, window);
                break;

            default:
                break;
        }

        VT_CORE_ASSERT(result != nullptr);
        return result;
    }
}