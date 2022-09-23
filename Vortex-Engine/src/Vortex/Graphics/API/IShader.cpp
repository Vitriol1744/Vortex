//
// Created by Vitriol1744 on 10.09.2022.
//
#include "Vortex/vtpch.hpp"
#include "IShader.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanShader.hpp"

namespace Vortex::Graphics
{
    Ref<IShader> IShader::Create(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, bool binaries)
    {
        Ref<IShader> result = nullptr;
        switch (IRendererAPI::GetGraphicsAPI())
        {
            case GraphicsAPI::eVulkan:
                result = CreateRef<VulkanShader>(vertexShaderPath, fragmentShaderPath, binaries);
                break;

            default:
                break;
        }

        VT_CORE_ASSERT_MSG(result != nullptr, "Failed to create Shader Program!");
        return result;
    }
}