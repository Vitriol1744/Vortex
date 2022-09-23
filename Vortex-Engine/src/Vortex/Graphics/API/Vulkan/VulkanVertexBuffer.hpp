//
// Created by Vitriol1744 on 08.09.2022.
//
#ifndef VORTEX_ENGINE_VKVERTEXBUFFER_HPP
#define VORTEX_ENGINE_VKVERTEXBUFFER_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IVertexBuffer.hpp"
#include "Vortex/Graphics/API/Vulkan/VulkanInstance.hpp"

namespace Vortex::Graphics
{
    class VT_API VulkanVertexBuffer : public IVertexBuffer
    {
        public:
            VulkanVertexBuffer(void* data, uint32 size, VertexBufferUsage usage = VertexBufferUsage::eStatic);
            VulkanVertexBuffer(uint32 size, VertexBufferUsage usage = VertexBufferUsage::eDynamic);

            virtual ~VulkanVertexBuffer() override;

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


#endif //VORTEX_ENGINE_VKVERTEXBUFFER_HPP
