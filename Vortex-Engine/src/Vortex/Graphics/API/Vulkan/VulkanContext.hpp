/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Graphics/API/RendererContext.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanDevice.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{
    class VulkanContext : public RendererContext
    {
      public:
        VulkanContext(class IWindow* window);
        virtual ~VulkanContext();

        virtual void                 Present() override;
        virtual void                 OnResize(u32 width, u32 height) override;

        const VulkanDevice&          GetDevice() const { return m_Device; }
        // TODO(v1tr10l7) change to const later
        VulkanSwapChain&             GetSwapChain() { return m_SwapChain; }
        inline vk::CommandPool       GetCommandPool() { return m_CommandPool; }

        static const VulkanInstance& GetInstance()
        {
            VtCoreAssert(s_VulkanInstance);
            return s_VulkanInstance;
        }
        static const VulkanPhysicalDevice& GetPhysicalDevice()
        {
            return s_PhysicalDevice;
        }

      private:
        VulkanDevice                m_Device;
        vk::PipelineCache           m_PipelineCache = VK_NULL_HANDLE;
        VulkanSwapChain             m_SwapChain;
        vk::CommandPool             m_CommandPool = VK_NULL_HANDLE;

        static usize                s_ContextCount;
        static VulkanInstance       s_VulkanInstance;
        static VulkanPhysicalDevice s_PhysicalDevice;
    };
}; // namespace Vortex
