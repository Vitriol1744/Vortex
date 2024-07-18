/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#ifdef VT_DEBUG
    #define VkCall(x)                                                          \
        {                                                                      \
            VkResult result = (x);                                             \
            if (result != VK_SUCCESS)                                          \
            {                                                                  \
                VtCoreFatal("{} != VK_SUCCESS! Error Code: {}", #x,            \
                            TranslateVkResult(result).data());                 \
            }                                                                  \
        }
#else
    #define VkCall(x) (x)
#endif

namespace Vortex
{
    inline static std::string TranslateVkResult(VkResult vkResult)
    {
        (void)vkResult;
        // TODO(v1tr10l7): TranslateVkResult
        return "TranslateVkResult: Not implemented";
    }
}; // namespace Vortex
