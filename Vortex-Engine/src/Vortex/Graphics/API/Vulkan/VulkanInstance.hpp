//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_VULKANINSTANCE_HPP
#define VORTEX_VULKANINSTANCE_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanInstance
    {
        public:
            inline VulkanInstance() = default;
            inline VulkanInstance(strview appName, uint version_Major, uint version_Minor, uint version_Patch, bool useValidationLayers)
            {
                Create(appName, version_Major, version_Minor, version_Patch, useValidationLayers);
            }
            inline ~VulkanInstance()
            {
                if (debugMessenger) DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
                if (instance) vkDestroyInstance(instance, nullptr);
            }

            void Create(strview appName, uint version_Major, uint version_Minor, uint version_Patch, bool useValidationLayers);

            static void LoadDevice(VkDevice device);

            VkInstance instance;

        private:
            VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

            static VkBool32 ValidationLayersSupported();

            void SetupDebugMessenger();
            static VkDebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo();

            VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
            void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    };
}


#endif //VORTEX_VULKANINSTANCE_HPP
