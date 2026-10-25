/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Layer.hpp>

#include <Vortex/Renderer/RHI/ImGuiLayer.hpp>
#include <Vortex/Renderer/RHI/Vulkan/Export.hpp>

#include <vulkan/vulkan.hpp>

namespace Vortex
{
    class VT_RENDERER_RHI_VULKAN_API VulkanImGuiLayer final : public ImGuiLayer
    {
      public:
        VulkanImGuiLayer(StringView name, RenderTargetWindow& mainWindow,
                         EventBus& eventBus)
            : ImGuiLayer(name, mainWindow, eventBus)
        {
        }
        ~VulkanImGuiLayer() = default;

        virtual void OnUpdate(Timestep deltaTime) override;
        virtual void OnRender() override;
        virtual void OnImGuiRender() override;

        virtual void OnInitializeRendererInterface() override;
        virtual void OnShutdownRendererInterface() override;

        virtual void OnNewFrame() override;
        virtual void OnRenderDrawData() override;

      private:
        vk::DescriptorPool        m_DescriptorPool = VK_NULL_HANDLE;
        vk::CommandPool           m_CommandPool    = VK_NULL_HANDLE;
        Vector<vk::CommandBuffer> m_CommandBuffers;

        static i32                CreateVulkanSurface(ImGuiViewport* viewport,
                                                      ImU64          vkInstance,
                                                      const void*    vkAllocator,
                                                      ImU64*         outVkSurface);
    };
}; // namespace Vortex
