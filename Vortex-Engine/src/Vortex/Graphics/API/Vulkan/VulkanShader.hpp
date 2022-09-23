//
// Created by Vitriol1744 on 15.09.2022.
//
#ifndef VORTEX_VULKANSHADER_HPP
#define VORTEX_VULKANSHADER_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IShader.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanShader : public IShader
    {
        public:
            VulkanShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath, bool binaries);
            ~VulkanShader();

            virtual const std::string& GetName() const noexcept override { return name; }
            inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() const noexcept { return shaderStages; }

            static std::vector<char> ReadFile(std::string_view filepath);
            static VkShaderModule CreateShaderModule(std::vector<char>& shaderCode);

        private:
            std::string name = "No Name";
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

            VkShaderModule vertexShader;
            VkShaderModule fragmentShader;
    };
}

#endif //VORTEX_VULKANSHADER_HPP
