/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>

import Prism.Core.Version;
import Prism.Utility.Expected;

namespace Vortex::Vulkan
{
    /// The minimum Vulkan version the backend requires (dynamic rendering and
    /// synchronization2 are core in 1.3).
    inline constexpr u32 kRequiredApiVersion = VK_API_VERSION_1_3;

    class VulkanInstance final
    {
      public:
        VulkanInstance() = default;
        VulkanInstance(const VulkanInstance&)            = delete;
        VulkanInstance& operator=(const VulkanInstance&) = delete;
        ~VulkanInstance() { Destroy(); }

        /// Loads Vulkan, creates the instance and (with `validation`) the
        /// validation layers' debug messenger.
        Expected<void, String> Initialize(StringView     applicationName,
                                          const Version& applicationVersion,
                                          StringView     engineName,
                                          const Version& engineVersion,
                                          bool           validation);
        void                   Destroy();

        inline                 operator vk::Instance() const { return m_Instance; }
        inline vk::Instance    Handle() const { return m_Instance; }
        inline bool            ValidationEnabled() const { return m_Validation; }
        inline bool            DebugUtilsEnabled() const { return m_DebugUtils; }

      private:
        vk::Instance               m_Instance       = VK_NULL_HANDLE;
        vk::DebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        bool                       m_Validation     = false;
        bool                       m_DebugUtils     = false;
    };
} // namespace Vortex::Vulkan
