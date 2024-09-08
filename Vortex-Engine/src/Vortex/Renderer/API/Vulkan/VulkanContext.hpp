/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/RendererContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanSwapChain.hpp"

namespace Vortex
{
    class VulkanContext : public RendererContext
    {
      public:
        VulkanContext(class Window* window, bool vsync);
        virtual ~VulkanContext();

        virtual void Present() override;
        virtual void OnResize() override;

        virtual void SetVSync(bool vsync) override
        {
            m_SwapChain.SetVSync(vsync);
        }

        VulkanSwapChain& GetSwapChain() { return m_SwapChain; }

      private:
        VulkanSwapChain m_SwapChain;
    };
}; // namespace Vortex
