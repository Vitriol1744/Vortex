/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/RendererContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanDevice.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanInstance.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{
    class VulkanContext : public RendererContext
    {
      public:
        VulkanContext(class Window* window);
        virtual ~VulkanContext();

        virtual void                 Present() override;
        virtual void                 OnResize(u32 width, u32 height) override;

        VulkanSwapChain&             GetSwapChain() { return m_SwapChain; }

        static const VulkanInstance& GetInstance()
        {
            VtCoreAssert(s_VulkanInstance);
            return s_VulkanInstance;
        }
        static const VulkanPhysicalDevice& GetPhysicalDevice()
        {
            return s_PhysicalDevice;
        }
        static const VulkanDevice& GetDevice() { return s_Device; }

      private:
        VulkanSwapChain             m_SwapChain;

        static usize                s_ContextCount;
        static VulkanInstance       s_VulkanInstance;
        static VulkanPhysicalDevice s_PhysicalDevice;
        static VulkanDevice         s_Device;
    };
}; // namespace Vortex
