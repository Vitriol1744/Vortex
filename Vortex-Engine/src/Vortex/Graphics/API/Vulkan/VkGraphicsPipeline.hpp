//
// Created by Vitriol1744 on 04.09.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/Pipeline.hpp"
#include "Vortex/Graphics/API/Vulkan/VkCommon.hpp"

namespace Vortex::Graphics
{
    class VT_API VkGraphicsPipeline : public Pipeline
    {
        public:
            ~VkGraphicsPipeline();

            void Initialize(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader);

        private:
            vk::Pipeline graphicsPipeline;
            vk::PipelineLayout pipelineLayout;
    };
}