//
// Created by Vitriol1744 on 10.09.2022.
//
#ifndef VORTEX_IGRAPHICSPIPELINE_HPP
#define VORTEX_IGRAPHICSPIPELINE_HPP

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IShader.hpp"
#include "Vortex/Graphics/RenderPass.hpp"
#include "Vortex/Graphics/VertexBufferLayout.hpp"
#include "Vortex/Graphics/Window/IWindow.hpp"

namespace Vortex::Graphics
{
    struct GraphicsPipelineCreateInfo
    {
        VertexBufferLayout layout;
        Ref<IShader> shader;
    };

    class VT_API IGraphicsPipeline
    {
        public:
            virtual ~IGraphicsPipeline() = default;
            virtual void Bind() = 0;

            static Ref<IGraphicsPipeline> Create(GraphicsPipelineCreateInfo& createInfo, Ref<IWindow> window);
    };
}

#endif //VORTEX_IGRAPHICSPIPELINE_HPP
