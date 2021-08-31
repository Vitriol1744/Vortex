//
// Created by vitriol1744 on 27.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/Vulkan/VkCommon.hpp"

namespace Vortex::Graphics
{
    class VT_API VkInstance
    {
        public:
            inline VkInstance() = default;
            inline VkInstance(strview appName, uint version_Major, uint version_Minor, uint version_Patch, bool useValidationLayers)
            {
                Create(appName, version_Major, version_Minor, version_Patch, useValidationLayers);
            }
            inline ~VkInstance()
            {
                if (debugMessenger) instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr);
                if (instance) instance.destroy(VkAllocator::Get().callbacks);
            }

            inline void Create() { Create("Vortex Application", 1, 0, 0); }
            void Create(strview appName, uint version_Major, uint version_Minor, uint version_Patch, bool useValidationLayers = true);

            vk::Instance instance;

        private:
            vk::DebugUtilsMessengerEXT debugMessenger;

            static vk::Bool32 ValidationLayersSupported();

            void SetupDebugMessenger();
            static vk::DebugUtilsMessengerCreateInfoEXT GetMessengerCreateInfo();
    };
}