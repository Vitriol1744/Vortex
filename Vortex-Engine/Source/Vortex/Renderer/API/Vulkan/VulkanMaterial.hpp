/*
 * Created by v1tr10l7 on 22.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/UnorderedMap.hpp>

#include <Vortex/Renderer/API/Material.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanShader.hpp>

namespace Vortex
{
    class VulkanMaterial : public Material
    {
      public:
        struct DescriptorSet
        {
            vk::DescriptorPool                           DescriptorPool;
            Vector<vk::DescriptorSet>                    DescriptorSets;

            UnorderedMap<String, vk::WriteDescriptorSet> WriteDescriptorSets;
        };

        VulkanMaterial(::Ref<Shader> shader);
        ~VulkanMaterial();

        inline const auto& DescriptorSets() const { return m_DescriptorSets; }

        virtual void Set(const String& name, ::Ref<Texture2D> texture) override;
        virtual void Set(const String&        name,
                         ::Ref<UniformBuffer> buffer) override;

      private:
        ::Ref<VulkanShader>              m_Shader = nullptr;
        UnorderedMap<u32, DescriptorSet> m_DescriptorSets;
    };
}; // namespace Vortex
