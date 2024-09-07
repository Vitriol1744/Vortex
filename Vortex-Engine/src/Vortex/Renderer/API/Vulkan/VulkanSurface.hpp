/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Window/Window.hpp"

namespace Vortex
{
    class VulkanSurface final
    {
      public:
        void   Create(Window* window, vk::PhysicalDevice physicalDevice);
        void   Destroy();

        inline operator vk::SurfaceKHR() const { return m_Surface; }
        inline const vk::SurfaceFormatKHR& GetFormat() const
        {
            return m_SurfaceFormat;
        }
        const vk::SurfaceCapabilitiesKHR&             GetCapabilities();
        inline const std::vector<vk::PresentModeKHR>& GetPresentModes() const
        {
            return m_PresentModes;
        }

        inline Window* GetWindow() const { return m_WindowHandle; }

      private:
        Window*                         m_WindowHandle;

        vk::SurfaceKHR                  m_Surface             = VK_NULL_HANDLE;
        vk::SurfaceFormatKHR            m_SurfaceFormat       = {};
        vk::SurfaceCapabilitiesKHR      m_SurfaceCapabilities = {};
        std::vector<vk::PresentModeKHR> m_PresentModes;
    };
} // namespace Vortex
