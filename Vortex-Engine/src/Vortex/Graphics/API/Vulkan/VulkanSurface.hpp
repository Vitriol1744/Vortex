/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"
#include "Vortex/Graphics/Window/IWindow.hpp"

namespace Vortex
{
    class VulkanSurface final
    {
      public:
        VulkanSurface(const VulkanInstance& instance, IWindow* window);
        ~VulkanSurface();

        void   Initialize(vk::PhysicalDevice physDevice);

        inline operator vk::SurfaceKHR() const { return m_Surface; }
        inline const vk::SurfaceFormatKHR& GetFormat() const
        {
            return m_SurfaceFormat;
        }
        inline const vk::SurfaceCapabilitiesKHR& GetCapabilities() const
        {
            return m_SurfaceCapabilities;
        }

      private:
        const VulkanInstance&      m_VulkanInstance;

        vk::SurfaceKHR             m_Surface             = VK_NULL_HANDLE;
        vk::SurfaceFormatKHR       m_SurfaceFormat       = {};
        vk::SurfaceCapabilitiesKHR m_SurfaceCapabilities = {};
    };
} // namespace Vortex
