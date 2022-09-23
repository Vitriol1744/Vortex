//
// Created by Vitriol1744 on 10.09.2022.
//
#include "VulkanCommon.hpp"

namespace Vortex::Graphics
{
    std::string TranslateVkResult(VkResult vkResult)
    {
        std::string result = "No Error";
        switch (vkResult)
        {
            case VK_SUCCESS:
                result = "Success!";
                break;
            case VK_NOT_READY:
                result = "A fence or query has not yet completed!";
                break;
            case VK_TIMEOUT:
                result = "A wait operation has not completed in the specified time!";
                break;
            case VK_EVENT_SET:
                result = "An event is signaled!";
                break;
            case VK_EVENT_RESET:
                result = "An event is unsignaled!";
                break;
            case VK_INCOMPLETE:
                result = "A return array was too small for the result!";
                break;
            case VK_SUBOPTIMAL_KHR:
                result = "A swapchain no longer matches the surface properties exactly, "
                         "but can still be used to present to the surface successfully!";
                break;
            case VK_THREAD_IDLE_KHR:
                result = "A deferred operation is not complete, "
                         "but there is currently no work for this thread to do at the time of this call!";
                break;
            case VK_THREAD_DONE_KHR:
                result = "A deferred operation is not complete but there is no work remaining to assign to additional threads!";
                break;
            case VK_OPERATION_DEFERRED_KHR:
                result = "A deferred operation was requested and at least some of the work was deferred!";
                break;
            case VK_OPERATION_NOT_DEFERRED_KHR:
                result = "A deferred operation was requested and no operations were deferred!";
                break;
            case VK_PIPELINE_COMPILE_REQUIRED:
                result = "A requested pipeline creation would have required compilation, "
                         "but the application requested compilation to not be performed!";
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                result = "A host memory allocation has failed!";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                result = "A device memory allocation has failed!";
                break;
            case VK_ERROR_INITIALIZATION_FAILED:
                result = "Initialization of an object could not be completed for implementation-specific reasons!";
                break;
            case VK_ERROR_DEVICE_LOST:
                result = "The logical or physical device has been lost. See Lost Devic!";
                break;
            case VK_ERROR_MEMORY_MAP_FAILED:
                result = "Mapping of a memory object has failed!";
                break;
            case VK_ERROR_LAYER_NOT_PRESENT:
                result = "A requested layer is not present or could not be loaded!";
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                result = "A requested extension is not supported!";
                break;
            case VK_ERROR_FEATURE_NOT_PRESENT:
                result = "A requested feature is not supported!";
                break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                result = "The requested version of Vulkan is not supported by the driver or "
                         "is otherwise incompatible for implementation-specific reasons!";
                break;
            case VK_ERROR_TOO_MANY_OBJECTS:
                result = "Too many objects of the type have already been created!";
                break;
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                result = "A requested format is not supported on this device!";
                break;
            case VK_ERROR_FRAGMENTED_POOL:
                result = "A pool allocation has failed due to fragmentation of the pool’s memory!";
                break;
            case VK_ERROR_SURFACE_LOST_KHR:
                result = "A surface is no longer available!";
                break;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                result = "The requested window is already in use by Vulkan or "
                         "another API in a manner which prevents it from being used again!";
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                result = "A surface has changed in such a way that it is no longer compatible with the swapchain, "
                         "and further presentation requests using the swapchain will fail!";
                break;
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                result = "The display used by a swapchain does not use the same presentable image layout, "
                         "or is incompatible in a way that prevents sharing an image!";
                break;
            case VK_ERROR_INVALID_SHADER_NV:
                result = "One or more shaders failed to compile or link. "
                         "More details are reported back to the application via VK_EXT_debug_report if enabled!";
                break;
            case VK_ERROR_OUT_OF_POOL_MEMORY:
                result = "A pool memory allocation has failed!";
                break;
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                result = "An external handle is not a valid handle of the specified type!";
                break;
            case VK_ERROR_FRAGMENTATION:
                result = "A descriptor pool creation has failed due to fragmentation!";
                break;
            case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
                result = "A buffer creation failed because the requested address is not available!";
                break;

            case VK_ERROR_UNKNOWN:
                result = "An unknown error has occurred; "
                         "either the application has provided invalid input, or an implementation failure has occurred.";
                break;

            default:
                result = "Unknown VkResult!";
                break;
        }

        return result;
    }
}
