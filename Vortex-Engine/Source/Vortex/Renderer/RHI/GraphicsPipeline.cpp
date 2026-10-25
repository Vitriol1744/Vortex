/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/GraphicsPipeline.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanGraphicsPipeline.hpp>

namespace Vortex
{
    ::Ref<GraphicsPipeline>
    GraphicsPipeline::Create(const GraphicsPipelineSpecification& specification)
    {
        return CreateRef<VulkanGraphicsPipeline>(specification);
    }
}; // namespace Vortex
