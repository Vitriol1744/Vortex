/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include "Vortex/Renderer/API/Vulkan/VulkanCommon.hpp"

namespace Vortex
{
    class VulkanInstance final
    {
      public:
        VulkanInstance() = default;

        void       Initialize();
        void       Destroy();

        inline u32 GetUsedApiVersion() const { return m_UsedApiVersion; }

        inline     operator vk::Instance() const { return m_Instance; }
        inline     operator bool() const { return m_Instance; }

        static const std::vector<const char*>& GetValidationLayers();
        static vk::Bool32                      ShouldUseValidationLayers();

      private:
        vk::Instance               m_Instance = VK_NULL_HANDLE;
        vk::DebugUtilsMessengerEXT m_DebugMessenger;
        u32                        m_UsedApiVersion = 0;

        void                       SetupDebugMessenger();

        static u32                 GetVulkanVersion();
        static vk::Bool32          ValidationLayersSupported();
        static vk::DebugUtilsMessengerCreateInfoEXT
        GetDebugMessengerCreateInfo();
    };
}; // namespace Vortex
