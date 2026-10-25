/*
 * Created by v1tr10l7 on 22.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Material.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanMaterial.hpp>

namespace Vortex
{
    ::Ref<Material> Material::Create(const ::Ref<Shader> shader)
    {
        return CreateRef<VulkanMaterial>(shader);
    }
} // namespace Vortex
