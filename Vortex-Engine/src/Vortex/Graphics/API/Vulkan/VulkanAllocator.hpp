//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_VULKANALLOCATOR_HPP
#define VORTEX_VULKANALLOCATOR_HPP

#include "vk_mem_alloc.h"

#include "Vortex/Core/Core.hpp"

namespace Vortex::Graphics
{
    struct VulkanAllocator
    {
        static VulkanAllocator& Get()
        {
            static VulkanAllocator allocator;

            return allocator;
        }

        static void InitializeVMA();

        VkAllocationCallbacks* callbacks = nullptr;
        VmaAllocator allocator;

        private:
            VulkanAllocator();
    };
}


#endif //VORTEX_VULKANALLOCATOR_HPP
