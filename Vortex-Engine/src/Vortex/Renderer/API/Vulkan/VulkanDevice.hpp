/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Core/Core.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanInstance.hpp"

namespace Vortex
{
    class VulkanPhysicalDevice final
    {
      public:
        struct QueueFamilyIndices
        {
            std::optional<u32> Graphics;
            std::optional<u32> Present;
            std::optional<u32> Compute;
            std::optional<u32> Transfer;
        };

        VulkanPhysicalDevice() = default;

        static VulkanPhysicalDevice Pick();

        inline operator vk::PhysicalDevice() const { return m_PhysicalDevice; }
        inline const QueueFamilyIndices& GetQueueFamilyIndices() const noexcept
        {
            return m_QueueFamilyIndices;
        }

        u32 FindMemoryType(u32                     typeFilter,
                           vk::MemoryPropertyFlags properties) const;

      private:
        vk::PhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        QueueFamilyIndices m_QueueFamilyIndices;
        usize              m_PhysicalDeviceVRAM = 0;

        void               FindQueueFamilies();
        vk::Bool32         IsDeviceSuitable(vk::PhysicalDevice physDevice);
        vk::Bool32 CheckDeviceExtensionSupport(vk::PhysicalDevice physDevice);
    };

    class VulkanDevice final
    {
      public:
        VulkanDevice() = default;

        void Initialize(const VulkanPhysicalDevice& physicalDevice);
        void Destroy();

        inline const VulkanPhysicalDevice& GetPhysicalDevice() const
        {
            return m_PhysicalDevice;
        }
        inline           operator vk::Device() const { return m_Device; }

        inline vk::Queue GetGraphicsQueue() const { return m_GraphicsQueue; }
        inline vk::Queue GetPresentQueue() const { return m_PresentQueue; }
        inline vk::Queue GetComputeQueue() const { return m_ComputeQueue; }
        inline vk::Queue GetTransferQueue() const { return m_TransferQueue; }

      private:
        VulkanPhysicalDevice m_PhysicalDevice;

        vk::Device           m_Device        = VK_NULL_HANDLE;
        vk::Queue            m_GraphicsQueue = VK_NULL_HANDLE;
        vk::Queue            m_PresentQueue  = VK_NULL_HANDLE;
        vk::Queue            m_ComputeQueue  = VK_NULL_HANDLE;
        vk::Queue            m_TransferQueue = VK_NULL_HANDLE;
    };
}; // namespace Vortex
