//
// Created by Vitriol1744 on 18.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IIndexBuffer.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanIndexBuffer.hpp"

namespace Vortex::Graphics
{
    Ref<IIndexBuffer> IIndexBuffer::Create(uint16* indices, uint32 size)
    {
        Ref<IIndexBuffer> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::eVulkan:
                result = CreateRef<VulkanIndexBuffer>(indices, size);
                break;

            default:
                break;
        }

        return result;
    }
}