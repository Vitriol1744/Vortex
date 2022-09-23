//
// Created by Vitriol1744 on 18.09.2022.
//
#ifndef VORTEX_VULKANINDEXBUFFER_HPP
#define VORTEX_VULKANINDEXBUFFER_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IIndexBuffer.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanIndexBuffer : public IIndexBuffer
    {
        public:
            VulkanIndexBuffer(uint16* data, uint32 size);

            virtual ~VulkanIndexBuffer() override;

            virtual void SetData(void* buffer, uint32 size, uint32 offset = 0) override { }

            virtual void Bind() const override { }

            virtual uint32 GetSize() const override { return size; }

            VkBuffer GetVulkanBuffer() const { return buffer; }

        private:
            uint32 size = 0;

            VkBuffer buffer;
            VkDeviceMemory bufferMemory;
            VmaAllocation allocation;
    };
}

#endif //VORTEX_VULKANINDEXBUFFER_HPP
