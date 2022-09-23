//
// Created by Vitriol1744 on 19.09.2022.
//
#ifndef VORTEX_VULKANUNIFORMBUFFER_HPP
#define VORTEX_VULKANUNIFORMBUFFER_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IUniformBuffer.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanUniformBuffer : public IUniformBuffer
    {
        public:
            VulkanUniformBuffer(size_t size);
            ~VulkanUniformBuffer();

            virtual void SetData(void* data) override;

        private:
            std::vector<VkBuffer> buffers;
            std::vector<VmaAllocation> bufferAllocations;
            size_t size;
    };
}
#endif //VORTEX_VULKANUNIFORMBUFFER_HPP
