/*
 * Created by v1tr10l7 on 22.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/API/Vulkan/VulkanMaterial.hpp>
#include <Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp>
#include <Vortex/Renderer/Renderer.hpp>

namespace Vortex
{
    VulkanMaterial::VulkanMaterial(::Ref<Shader> shader)
    {
        m_Shader                  = shader.As<VulkanShader>();
        u32        framesInFlight = Renderer::Configuration().MaxFramesInFlight;

        vk::Device device         = VulkanRenderer::Device();
        for (auto& set : m_Shader->DescriptorSets())
        {
            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
            poolInfo.pNext   = VK_NULL_HANDLE;
            poolInfo.flags   = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
            poolInfo.maxSets = framesInFlight;
            poolInfo.poolSizeCount = set.Value.PoolSizes.Size();
            poolInfo.pPoolSizes    = set.Value.PoolSizes.Raw();

            vk::DescriptorPool& descriptorPool
                = m_DescriptorSets[set.Key].DescriptorPool;
            VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                               &descriptorPool));

            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
            allocInfo.pNext = VK_NULL_HANDLE;
            allocInfo.descriptorPool     = descriptorPool;
            allocInfo.descriptorSetCount = framesInFlight;

            Vector<vk::DescriptorSetLayout> layouts(
                framesInFlight, m_Shader->DescriptorSetLayouts()[set.Key]);
            allocInfo.pSetLayouts = layouts.Raw();

            m_DescriptorSets[set.Key].DescriptorSets.Resize(framesInFlight);
            VkCall(device.allocateDescriptorSets(
                &allocInfo, m_DescriptorSets[set.Key].DescriptorSets.Raw()));

            for (auto& [key, value] : set.Value.WriteDescriptorSets)
                m_DescriptorSets[set.Key]
                    .WriteDescriptorSets[StringView(key.Raw())]
                    = value;

            for (auto& writeSet : m_DescriptorSets[set.Key].WriteDescriptorSets)
                writeSet.Value.dstSet = VK_NULL_HANDLE;
        }
    }

    VulkanMaterial::~VulkanMaterial()
    {
        vk::Device device = VulkanRenderer::Device();
        for (auto& set : m_DescriptorSets)
        {
            (void)device.freeDescriptorSets(set.Value.DescriptorPool,
                                            set.Value.DescriptorSets.Size(),
                                            set.Value.DescriptorSets.Raw());

            (void)device.destroyDescriptorPool(set.Value.DescriptorPool);
        }
    }

    void VulkanMaterial::Set(const String& name, ::Ref<Texture2D> texture)
    {
        vk::Device              device = VulkanRenderer::Device();
        DescriptorSet&          set    = m_DescriptorSets[0];
        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];

        auto vulkanTexture     = texture.As<VulkanTexture2D>();
        u32  maxFramesInFlight = Renderer::Configuration().MaxFramesInFlight;
        for (usize i = 0; i < maxFramesInFlight; i++)
        {
            descriptorWrite.dstSet          = set.DescriptorSets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType
                = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrite.pImageInfo = &vulkanTexture->ImageInfo();

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }
    void VulkanMaterial::Set(const String& name, ::Ref<UniformBuffer> uniform)
    {
        vk::Device              device = VulkanRenderer::Device();
        DescriptorSet&          set    = m_DescriptorSets[0];
        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];
        auto buffer = uniform.As<VulkanUniformBuffer>();

        for (usize i = 0; i < Renderer::Configuration().MaxFramesInFlight; i++)
        {
            descriptorWrite.dstSet          = set.DescriptorSets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.pBufferInfo    = &buffer->BufferInfos()[i];

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }
}; // namespace Vortex
