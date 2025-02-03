/*
 * Created by v1tr10l7 on 10.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Framebuffer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanFramebuffer.hpp"

namespace Vortex
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specs)
    {
        return CreateRef<VulkanFramebuffer>(specs);
    }
}; // namespace Vortex
