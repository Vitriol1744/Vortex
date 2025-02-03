/*
 * Created by v1tr10l7 on 22.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanMaterial.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/Renderer.hpp"

namespace Vortex
{
    VulkanMaterial::VulkanMaterial(Ref<Shader> shader)
    {
        m_Shader           = std::dynamic_pointer_cast<VulkanShader>(shader);
        u32 framesInFlight = Renderer::GetConfiguration().MaxFramesInFlight;

        vk::Device device  = VulkanRenderer::GetDevice();
        for (auto& set : m_Shader->GetDescriptorSets())
        {
            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType   = vk::StructureType::eDescriptorPoolCreateInfo;
            poolInfo.pNext   = VK_NULL_HANDLE;
            poolInfo.flags   = vk::DescriptorPoolCreateFlags();
            poolInfo.maxSets = framesInFlight;
            poolInfo.poolSizeCount = set.second.PoolSizes.size();
            poolInfo.pPoolSizes    = set.second.PoolSizes.data();

            vk::DescriptorPool& descriptorPool
                = m_DescriptorSets[set.first].DescriptorPool;
            VkCall(device.createDescriptorPool(&poolInfo, VK_NULL_HANDLE,
                                               &descriptorPool));

            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
            allocInfo.pNext = VK_NULL_HANDLE;
            allocInfo.descriptorPool     = descriptorPool;
            allocInfo.descriptorSetCount = framesInFlight;

            std::vector<vk::DescriptorSetLayout> layouts(
                framesInFlight, m_Shader->GetDescriptorSetLayouts()[set.first]);
            allocInfo.pSetLayouts = layouts.data();

            m_DescriptorSets[set.first].DescriptorSets.resize(framesInFlight);
            VkCall(device.allocateDescriptorSets(
                &allocInfo, m_DescriptorSets[set.first].DescriptorSets.data()));

            m_DescriptorSets[set.first].WriteDescriptorSets
                = set.second.WriteDescriptorSets;

            for (auto& writeSet :
                 m_DescriptorSets[set.first].WriteDescriptorSets)
                writeSet.second.dstSet = VK_NULL_HANDLE;
        }
    }

    VulkanMaterial::~VulkanMaterial()
    {
        vk::Device device = VulkanRenderer::GetDevice();
        for (auto& set : m_DescriptorSets)
        {
            device.freeDescriptorSets(set.second.DescriptorPool,
                                      set.second.DescriptorSets.size(),
                                      set.second.DescriptorSets.data());

            device.destroyDescriptorPool(set.second.DescriptorPool);
        }
    }

    void VulkanMaterial::Set(const std::string& name, Ref<Texture2D> texture)
    {
        vk::Device              device = VulkanRenderer::GetDevice();
        DescriptorSet&          set    = m_DescriptorSets[0];
        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];

        auto vulkanTexture
            = std::dynamic_pointer_cast<VulkanTexture2D>(texture);

        u32 maxFramesInFlight = Renderer::GetConfiguration().MaxFramesInFlight;
        for (usize i = 0; i < maxFramesInFlight; i++)
        {
            descriptorWrite.dstSet          = set.DescriptorSets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType
                = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrite.pImageInfo = &vulkanTexture->GetImageInfo();

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }
    void VulkanMaterial::Set(const std::string& name,
                             Ref<UniformBuffer> uniform)
    {
        vk::Device              device = VulkanRenderer::GetDevice();
        DescriptorSet&          set    = m_DescriptorSets[0];
        vk::WriteDescriptorSet& descriptorWrite
            = m_DescriptorSets[0].WriteDescriptorSets[name];
        auto buffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(uniform);

        for (usize i = 0; i < Renderer::GetConfiguration().MaxFramesInFlight;
             i++)
        {
            descriptorWrite.dstSet          = set.DescriptorSets[i];
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.pBufferInfo    = &buffer->GetBufferInfos()[i];

            device.updateDescriptorSets(1, &descriptorWrite, 0, VK_NULL_HANDLE);
        }
    }
}; // namespace Vortex
