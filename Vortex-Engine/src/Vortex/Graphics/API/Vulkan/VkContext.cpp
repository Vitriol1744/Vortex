//
// Created by vitriol1744 on 27.08.2021.
//
#include "VkContext.hpp"

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
    static uint32 contextsCount = 0;

    VkContext::VkContext(NativeWindowHandleType window)
    {
        windowSurface.Initialize(window);
        vk::Extent2D extent;
        if (windowSurface.GetSurfaceCapabilities().currentExtent.width != UINT32_MAX) extent = windowSurface.GetSurfaceCapabilities().currentExtent;
        else
        {
            extent.width = 800;
            extent.height = 600;
        }
        swapChain.Initialize(extent, windowSurface);
        contextsCount++;
    }
    VkContext::~VkContext()
    {
        contextsCount--;
    }

    void VkContext::Present()
    {
    
    }
    void VkContext::Activate()
    {
        
    }
}