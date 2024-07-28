/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Core/Layer.hpp"

namespace Vortex
{
    class VulkanImGuiLayer final : public Layer
    {
      public:
        VulkanImGuiLayer(std::string_view name)
            : Layer(name)
        {
        }
        ~VulkanImGuiLayer() = default;

        virtual void OnAttach();
        virtual void OnDetach();

        virtual void Begin();
        virtual void End();

        virtual void OnUpdate();
        virtual void OnRender();
        virtual void OnImGuiRender();

      private:
        vk::DescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    };
}; // namespace Vortex
