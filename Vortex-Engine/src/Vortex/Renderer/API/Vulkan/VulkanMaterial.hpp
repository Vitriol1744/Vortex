/*
 * Created by v1tr10l7 on 22.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Material.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

namespace Vortex
{
    class VulkanMaterial : public Material
    {
      public:
        struct DescriptorSet
        {
            vk::DescriptorPool             DescriptorPool;
            std::vector<vk::DescriptorSet> DescriptorSets;

            std::unordered_map<std::string, vk::WriteDescriptorSet>
                WriteDescriptorSets;
        };

        VulkanMaterial(Ref<Shader> shader);
        ~VulkanMaterial();

        inline const auto& GetDescriptorSets() const
        {
            return m_DescriptorSets;
        }

        virtual void Set(const std::string& name,
                         Ref<Texture2D>     texture) override;
        virtual void Set(const std::string& name,
                         Ref<UniformBuffer> buffer) override;

      private:
        Ref<VulkanShader>                      m_Shader = nullptr;
        std::unordered_map<u32, DescriptorSet> m_DescriptorSets;
    };
}; // namespace Vortex
