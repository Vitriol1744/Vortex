/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Span.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanCommon.hpp>

namespace Vortex
{
    class VulkanInstance final
    {
      public:
        VulkanInstance() = default;

        void       Initialize();
        void       Destroy();

        inline u32 UsedApiVersion() const { return m_UsedApiVersion; }

        inline     operator vk::Instance() const { return m_Instance; }
        inline     operator bool() const { return m_Instance; }

        static const Span<const char*> ValidationLayers();
        static vk::Bool32              ShouldUseValidationLayers();

      private:
        vk::Instance                                m_Instance = VK_NULL_HANDLE;
        vk::DebugUtilsMessengerEXT                  m_DebugMessenger;
        u32                                         m_UsedApiVersion = 0;

        void                                        SetupDebugMessenger();

        static u32                                  VulkanVersion();
        static vk::Bool32                           ValidationLayersSupported();
        static vk::DebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo();
    };
}; // namespace Vortex
