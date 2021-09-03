//
// Created by vitriol1744 on 28.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "Vortex/Graphics/API/Vulkan/VkAllocator.hpp"

#ifdef VT_DEBUG
#define VkCall(x, msg)\
    {\
        vk::Result result = (x);\
        if (result != vk::Result::eSuccess)\
        {\
            VTCoreLogFatal("{} Error Code: {}", msg, TranslateVkResult(result).data());\
        }\
    }
    
#define VkCall_(x)\
    {\
        vk::Result result = (x);\
        if (result != vk::Result::eSuccess)\
        {\
            VTCoreLogFatal("{} != VK_SUCCESS! Error Code: {}", #x, TranslateVkResult(result).data());\
        }\
    }
#else
    #define VkCall(x, msg) x
    #define VkCall_(x) x
#endif

namespace Vortex::Graphics
{
    std::string TranslateVkResult(vk::Result vkResult);
    template<typename T>
    inline T VkGetInstanceProcAddress(vk::Instance vkInstance, const char* name)
    {
        return reinterpret_cast<T>(vkInstance.getProcAddr(name));
    }
}