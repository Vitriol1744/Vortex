/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanSurface.hpp"

namespace Vortex
{
    class VulkanPhysicalDevice final
    {
      public:
        struct QueueFamilyIndices
        {
            std::optional<u64> Graphics;
            std::optional<u64> Present;
            std::optional<u64> Compute;
            std::optional<u64> Transfer;
        };

        VulkanPhysicalDevice(const VulkanInstance& instance,
                             const VulkanSurface&  surface);
        ~VulkanPhysicalDevice();

        inline operator vk::PhysicalDevice() const { return m_PhysicalDevice; }
        inline const QueueFamilyIndices& GetQueueFamilyIndices() const noexcept
        {
            return m_QueueFamilyIndices;
        }

      private:
        vk::PhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        QueueFamilyIndices m_QueueFamilyIndices;
        usize              m_PhysicalDeviceVRAM = 0;

        void               FindQueueFamilies(const VulkanSurface& surface);
        vk::Bool32         IsDeviceSuitable(vk::PhysicalDevice physDevice);
        vk::Bool32 CheckDeviceExtensionSupport(vk::PhysicalDevice physDevice);
    };
}; // namespace Vortex
