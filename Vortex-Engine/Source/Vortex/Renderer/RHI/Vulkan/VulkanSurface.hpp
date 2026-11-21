/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

import Prism.Containers.Vector;

namespace Vortex
{
    class Window;
}

namespace Vortex::Vulkan
{
    /// The presentation surface of a window.
    class VulkanSurface final
    {
      public:
        VulkanSurface() = default;
        VulkanSurface(const VulkanSurface&)            = delete;
        VulkanSurface& operator=(const VulkanSurface&) = delete;

        /// Creates the platform surface. Fails if the device's graphics queue
        /// can't present to it.
        bool                       Create(class Window* window);
        /// Hands the surface to the deferred deletion queue.
        void                       Release();
        /// Destroys it now (its swapchain must already be gone).
        void                       Destroy();

        inline vk::SurfaceKHR      Handle() const { return m_Surface; }
        inline class Window*       Window() const { return m_Window; }

        vk::SurfaceCapabilitiesKHR Capabilities() const;
        Vector<vk::SurfaceFormatKHR> Formats() const;
        Vector<vk::PresentModeKHR> PresentModes() const;

      private:
        class Window*  m_Window  = nullptr;
        vk::SurfaceKHR m_Surface = VK_NULL_HANDLE;
    };
} // namespace Vortex::Vulkan
