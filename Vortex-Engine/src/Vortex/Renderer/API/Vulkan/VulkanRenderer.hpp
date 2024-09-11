/*
 * Created by v1tr10l7 on 13.08.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/RendererBackend.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanDevice.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanInstance.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{
    class VulkanRenderer : public RendererBackend
    {
      public:
        VulkanRenderer();
        virtual ~VulkanRenderer() = default;

        virtual void  Initialize() override;
        virtual void  Shutdown() override;

        virtual void  BeginFrame(Window& window) override;
        virtual void  EndFrame() override;
        virtual void  BeginRenderPass(Ref<Framebuffer> target) override;
        virtual void  EndRenderPass() override;

        virtual void  Draw(Ref<GraphicsPipeline> pipeline,
                           Ref<VertexBuffer>     vertexBuffer,
                           Ref<IndexBuffer>      indexBuffer,
                           u32                   indexCount) override;

        virtual usize GetMemoryUsage() override;
        virtual usize GetMemoryBudget() override;

        inline static const VulkanInstance& GetVulkanInstance()
        {
            VtCoreAssert(s_VkInstance);
            return s_VkInstance;
        }
        inline static consteval const VulkanPhysicalDevice& GetPhysicalDevice()
        {
            return s_PhysicalDevice;
        }
        inline static consteval const VulkanDevice& GetDevice()
        {
            return s_Device;
        }

      private:
        Ref<VulkanSwapChain>                m_CurrentSwapChain = nullptr;
        vk::PhysicalDeviceMemoryProperties2 m_MemoryProperties{};
        vk::PhysicalDeviceMemoryBudgetPropertiesEXT m_MemoryBudgetProperties{};

        static VulkanInstance                       s_VkInstance;
        static VulkanPhysicalDevice                 s_PhysicalDevice;
        static VulkanDevice                         s_Device;
    };
}; // namespace Vortex
