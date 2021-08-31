//
// Created by vitriol1744 on 28.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include <vulkan/vulkan.hpp>

namespace Vortex::Graphics
{
    struct VkAllocator
    {
        static VkAllocator& Get()
        {
            static VkAllocator allocator;
            
            return allocator;
        }
        
        vk::AllocationCallbacks* callbacks = nullptr;
        
        private:
            inline VkAllocator()
            {
                //TODO: Create Callbacks!
            }
    };
}

