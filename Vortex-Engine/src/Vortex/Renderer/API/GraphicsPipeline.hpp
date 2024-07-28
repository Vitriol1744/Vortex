/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"

namespace Vortex
{
    struct GraphicsPipelineSpecification
    {
        Ref<class Window> Window = nullptr;
        Ref<class Shader> Shader = nullptr;
    };

    class VT_API GraphicsPipeline
    {
      public:
        GraphicsPipeline()          = default;
        virtual ~GraphicsPipeline() = default;

        static Ref<GraphicsPipeline>
        Create(const GraphicsPipelineSpecification&);
    };
}; // namespace Vortex
