//
// Created by Vitriol1744 on 11.09.2022.
//
#ifndef VORTEX_RENDERPASS_HPP
#define VORTEX_RENDERPASS_HPP

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    struct RenderPassAttachment
    {

    };

    enum class PipelineBindPoint
    {
        eGraphics
    };

    class RenderPass
    {
        public:
            struct SubPass
            {
                PipelineBindPoint pipelineBindPoint;
                std::vector<RenderPassAttachment> colorAttachments;
            };

            std::vector<RenderPassAttachment> attachments;
            std::vector<SubPass> subpasses;
    };
}

#endif //VORTEX_RENDERPASS_HPP
