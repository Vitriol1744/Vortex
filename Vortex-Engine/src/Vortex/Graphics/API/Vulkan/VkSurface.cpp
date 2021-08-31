//
// Created by vitriol1744 on 31.08.2021.
//
#include "VkSurface.hpp"

#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

#ifdef VT_PLATFORM_WINDOWS
    #include <Windows.h>
    #include <vulkan/vulkan_win32.h>
#elif defined(VT_PLATFORM_LINUX)
    #include "Vortex/Platform/X11/X11.hpp"
    #include <vulkan/vulkan_xlib.h>
    #include <Vortex/Graphics/Window/X11/X11Window.hpp>
#endif

namespace Vortex::Graphics
{
    VkSurface::~VkSurface()
    {
        VkRendererAPI::GetVulkanInstance().instance.destroySurfaceKHR(surface, VkAllocator::Get().callbacks);
    }

    void VkSurface::Initialize(Platform::NativeWindowHandleType windowHandle)
    {
        VkSurfaceKHR surface;
        //TODO: Choose Surface Format!

        #ifdef VT_PLATFORM_WINDOWS
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext     = VK_NULL_HANDLE;
            surfaceCreateInfo.flags     = 0;
            surfaceCreateInfo.hinstance = GetModuleHandleW(nullptr);
            surfaceCreateInfo.hwnd      = windowHandle;
            
            vkCreateWin32SurfaceKHR(VkRendererAPI::GetVulkanInstance().instance, &surfaceCreateInfo,
                                    reinterpret_cast<const VkAllocationCallbacks*>(VkAllocator::Get().callbacks), &surface);
        #elif defined(VT_PLATFORM_LINUX)
            VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext = VK_NULL_HANDLE;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.dpy = WindowImpl::GetDisplay();
            surfaceCreateInfo.window = windowHandle;

            vkCreateXlibSurfaceKHR(VkRendererAPI::GetVulkanInstance().instance, &surfaceCreateInfo,
                                   reinterpret_cast<const VkAllocationCallbacks*>(VkAllocator::Get().callbacks), &surface);
        #endif

        this->surface = surface;
        VkInstance& vkInstance = VkRendererAPI::GetVulkanInstance();
        VkRendererAPI::GetDevice().Initialize(vkInstance, *this);
    }
}
