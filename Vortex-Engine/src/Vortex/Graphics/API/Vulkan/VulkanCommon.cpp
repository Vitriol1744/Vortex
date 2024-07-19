/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

namespace Vortex::Vulkan
{
    std::string VkResultToString(vk::Result vkResult)
    {
        std::string result = "Unknown Error";
        switch (vkResult)
        {
            case vk::Result::eSuccess: result = "Success!"; break;
            case vk::Result::eNotReady:
                result = "A fence or query has not yet completed!";
                break;
            case vk::Result::eTimeout:
                result
                    = "A wait operation has not completed in the specified "
                      "time!";
                break;
            case vk::Result::eEventSet: result = "An event is signaled!"; break;
            case vk::Result::eEventReset:
                result = "An event is unsignaled!";
                break;
            case vk::Result::eIncomplete:
                result = "A return array was too small for the result!";
                break;
            case vk::Result::eSuboptimalKHR:
                result
                    = "A swapchain no longer matches the surface properties "
                      "exactly, "
                      "but can still be used to present to the surface "
                      "successfully!";
                break;
            case vk::Result::eThreadIdleKHR:
                result
                    = "A deferred operation is not complete, "
                      "but there is currently no work for this thread to do at "
                      "the time of this call!";
                break;
            case vk::Result::eThreadDoneKHR:
                result
                    = "A deferred operation is not complete but there is no "
                      "work remaining to assign to additional threads!";
                break;
            case vk::Result::eOperationDeferredKHR:
                result
                    = "A deferred operation was requested and at least some of "
                      "the work was deferred!";
                break;
            case vk::Result::eOperationNotDeferredKHR:
                result
                    = "A deferred operation was requested and no operations "
                      "were deferred!";
                break;
            case vk::Result::ePipelineCompileRequiredEXT:
                result
                    = "A requested pipeline creation would have required "
                      "compilation, "
                      "but the application requested compilation to not be "
                      "performed!";
                break;
            case vk::Result::eErrorOutOfHostMemory:
                result = "A host memory allocation has failed!";
                break;
            case vk::Result::eErrorOutOfDeviceMemory:
                result = "A device memory allocation has failed!";
                break;
            case vk::Result::eErrorInitializationFailed:
                result
                    = "Initialization of an object could not be completed for "
                      "implementation-specific reasons!";
                break;
            case vk::Result::eErrorDeviceLost:
                result
                    = "The logical or physical device has been lost. See Lost "
                      "Devic!";
                break;
            case vk::Result::eErrorMemoryMapFailed:
                result = "Mapping of a memory object has failed!";
                break;
            case vk::Result::eErrorLayerNotPresent:
                result
                    = "A requested layer is not present or could not be "
                      "loaded!";
                break;
            case vk::Result::eErrorExtensionNotPresent:
                result = "A requested extension is not supported!";
                break;
            case vk::Result::eErrorFeatureNotPresent:
                result = "A requested feature is not supported!";
                break;
            case vk::Result::eErrorIncompatibleDriver:
                result
                    = "The requested version of Vulkan is not supported by the "
                      "driver or "
                      "is otherwise incompatible for implementation-specific "
                      "reasons!";
                break;
            case vk::Result::eErrorTooManyObjects:
                result
                    = "Too many objects of the type have already been created!";
                break;
            case vk::Result::eErrorFormatNotSupported:
                result = "A requested format is not supported on this device!";
                break;
            case vk::Result::eErrorFragmentedPool:
                result
                    = "A pool allocation has failed due to fragmentation of "
                      "the pool’s memory!";
                break;
            case vk::Result::eErrorSurfaceLostKHR:
                result = "A surface is no longer available!";
                break;
            case vk::Result::eErrorNativeWindowInUseKHR:
                result
                    = "The requested window is already in use by Vulkan or "
                      "another API in a manner which prevents it from being "
                      "used again!";
                break;
            case vk::Result::eErrorOutOfDateKHR:
                result
                    = "A surface has changed in such a way that it is no "
                      "longer compatible with the swapchain, "
                      "and further presentation requests using the swapchain "
                      "will fail!";
                break;
            case vk::Result::eErrorIncompatibleDisplayKHR:
                result
                    = "The display used by a swapchain does not use the same "
                      "presentable image layout, "
                      "or is incompatible in a way that prevents sharing an "
                      "image!";
                break;
            case vk::Result::eErrorInvalidShaderNV:
                result
                    = "One or more shaders failed to compile or link. "
                      "More details are reported back to the application via "
                      "VK_EXT_debug_report if enabled!";
                break;
            case vk::Result::eErrorOutOfPoolMemory:
                result = "A pool memory allocation has failed!";
                break;
            case vk::Result::eErrorInvalidExternalHandle:
                result
                    = "An external handle is not a valid handle of the "
                      "specified type!";
                break;
            case vk::Result::eErrorFragmentation:
                result
                    = "A descriptor pool creation has failed due to "
                      "fragmentation!";
                break;
            case vk::Result::eErrorInvalidDeviceAddressEXT:
                result
                    = "A buffer creation failed because the requested address "
                      "is not available!";
                break;

            case vk::Result::eErrorUnknown:
                result
                    = "An unknown error has occurred; "
                      "either the application has provided invalid input, or "
                      "an implementation failure has occurred.";
                break;

            default: break;
        }

        return result;
    }
}; // namespace Vortex::Vulkan
