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

        u32        FindMemoryType(u32                     typeFilter,
                                  vk::MemoryPropertyFlags properties) const;
        vk::Format FindDepthFormat() const;

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

        inline vk::CommandPool GetGraphicsCommandPool() const
        {
            return m_GraphicsPool;
        }
        inline vk::CommandPool GetTransferCommandPool() const
        {
            return m_TransferPool;
        }

        vk::CommandBuffer
             BeginOneTimeCommand(vk::CommandPool commandPool) const;
        void EndOneTimeCommand(vk::CommandPool   commandPool,
                               vk::CommandBuffer commandBuffer,
                               vk::Queue         queue) const;

        vk::CommandBuffer BeginOneTimeRenderCommand() const
        {
            return BeginOneTimeCommand(m_GraphicsPool);
        }
        void EndOneTimeRenderCommand(vk::CommandBuffer commandBuffer) const
        {
            EndOneTimeCommand(m_GraphicsPool, commandBuffer, m_GraphicsQueue);
        }

        vk::CommandBuffer BeginTransferCommand() const
        {
            return BeginOneTimeCommand(m_TransferPool);
        }
        void EndTransferCommand(vk::CommandBuffer commandBuffer) const
        {
            EndOneTimeCommand(m_TransferPool, commandBuffer, m_TransferQueue);
        }

      private:
        VulkanPhysicalDevice m_PhysicalDevice;

        vk::Device           m_Device        = VK_NULL_HANDLE;
        vk::Queue            m_GraphicsQueue = VK_NULL_HANDLE;
        vk::Queue            m_PresentQueue  = VK_NULL_HANDLE;
        vk::Queue            m_ComputeQueue  = VK_NULL_HANDLE;
        vk::Queue            m_TransferQueue = VK_NULL_HANDLE;

        // Command pools for one time commands
        vk::CommandPool      m_GraphicsPool  = VK_NULL_HANDLE;
        vk::CommandPool      m_TransferPool  = VK_NULL_HANDLE;
    };
}; // namespace Vortex
