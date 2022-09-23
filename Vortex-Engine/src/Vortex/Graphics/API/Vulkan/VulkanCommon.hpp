//
// Created by Vitriol1744 on 10.09.2022.
//
#ifndef VORTEX_ENGINE_VKCOMMON_HPP
#define VORTEX_ENGINE_VKCOMMON_HPP

#define VT_EXPORT_VULKAN_HEADERS
#include "Vortex/vtpch.hpp"
#include "Vortex/Core/Core.hpp"

//#define VK_NO_PROTOTYPES
#include "Vortex/Graphics/API/Vulkan/VulkanAllocator.hpp"

#ifndef VT_DIST
    #define VkCall_msg(x, msg) { if (!CheckVkResult(x, msg)) VT_DEBUG_BREAK; }
    #define VkCall(x) { if (!CheckVkResult(x, #x)) VT_DEBUG_BREAK; }
    #define VkTry_msg(x, msg) { CheckVkResult(x, msg, false); }
    #define VkTry(x) { CheckVkResult(x, #x, false); }
#else
    #define VkCall_msg(x, msg) x
    #define VkCall(x) x
    #define VkTry_msg(x, msg) x
    #define VkTry(x) x
#endif

namespace Vortex::Graphics
{
    std::string TranslateVkResult(VkResult vkResult);
    template<typename T>
    inline T VkGetInstanceProcAddress(VkInstance vkInstance, const char* name)
    {
        return reinterpret_cast<T>(vkGetInstanceProcAddr(vkInstance, name));
    }
    inline bool CheckVkResult(VkResult result, std::string_view msg, bool fatal=true)
    {
        if (result != VK_SUCCESS)
        {
            fatal
            ? VTCoreLogFatal("Vulkan: {} != VK_SUCCESS! Error Code: {}", msg, TranslateVkResult(result).data())
            : VTCoreLogError("Vulkan: {} != VK_SUCCESS! Error Code: {}", msg, TranslateVkResult(result).data());
            return false;
        }
        return true;
    }
}

#endif //VORTEX_ENGINE_VKCOMMON_HPP
