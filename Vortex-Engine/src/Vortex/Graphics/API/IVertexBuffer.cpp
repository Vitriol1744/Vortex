//
// Created by Vitriol1744 on 16.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IVertexBuffer.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanVertexBuffer.hpp"

namespace Vortex::Graphics
{
    Ref<IVertexBuffer> IVertexBuffer::Create(void* vertices, uint32 size)
    {
        Ref<IVertexBuffer> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::eVulkan:
                result = CreateRef<VulkanVertexBuffer>(vertices, size);
                break;

            default:
                break;
        }

        return result;
    }
}