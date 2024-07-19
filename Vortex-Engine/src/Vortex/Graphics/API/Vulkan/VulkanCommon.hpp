/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#ifndef VT_DIST
    #define VkCall(x)                                                          \
        {                                                                      \
            vk::Result status = (x);                                           \
            if (status == vk::Result::eSuccess)                                \
                ;                                                              \
            else                                                               \
            {                                                                  \
                VtCoreFatal("Vulkan: '{}' != VK_SUCCESS\nError Code: {}", #x,  \
                            Vulkan::VkResultToString(status));                 \
            }                                                                  \
        }
#endif

namespace Vortex::Vulkan
{
    std::string VkResultToString(vk::Result result);

    template <typename T>
    inline T VkGetInstanceProcAddress(vk::Instance instance,
                                      const char*  procName)
    {
        return reinterpret_cast<T>(instance.getProcAddr(procName));
    }
}; // namespace Vortex::Vulkan
