/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Graphics/Window/IWindow.hpp"

#include <GLFW/glfw3.h>

namespace Vortex
{
    class VulkanSurface final
    {
      public:
        void   Create(IWindow* window, vk::PhysicalDevice physicalDevice);
        void   Destroy();

        inline operator vk::SurfaceKHR() const { return m_Surface; }
        inline const vk::SurfaceFormatKHR& GetFormat() const
        {
            return m_SurfaceFormat;
        }
        inline const vk::SurfaceCapabilitiesKHR& GetCapabilities() const
        {
            return m_SurfaceCapabilities;
        }
        inline const std::vector<vk::PresentModeKHR>& GetPresentModes() const
        {
            return m_PresentModes;
        }

        inline GLFWwindow* GetNativeWindowHandle() const
        {
            return m_WindowHandle;
        }

      private:
        GLFWwindow*                     m_WindowHandle;

        vk::SurfaceKHR                  m_Surface             = VK_NULL_HANDLE;
        vk::SurfaceFormatKHR            m_SurfaceFormat       = {};
        vk::SurfaceCapabilitiesKHR      m_SurfaceCapabilities = {};
        std::vector<vk::PresentModeKHR> m_PresentModes;
    };
} // namespace Vortex
