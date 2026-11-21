/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanDevice.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanPipeline.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanResources.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanShader.hpp>

import Prism.Containers.Vector;

namespace Vortex::Vulkan
{
    using namespace RHI;
    using Deletion = DeferredDeletion::Type;

    //------------------------------------------------------------------------
    // Bind groups -----------------------------------------------------------
    //------------------------------------------------------------------------

    VulkanBindGroup::VulkanBindGroup(const BindGroupDesc& desc)
        : BindGroup(desc)
    {
        VtCoreAssertMsg(desc.Layout, "Vulkan: BindGroup without a layout");
        auto& device = VulkanDevice::Get();
        auto* layout = static_cast<VulkanBindGroupLayout*>(desc.Layout.Raw());
        m_Set        = device.AllocateDescriptorSet(layout->Handle(), m_Pool);
        SetDebugName(device.Handle(), m_Set, desc.DebugName);

        // Info structs must stay at fixed addresses until the update call.
        Vector<vk::DescriptorBufferInfo> bufferInfos;
        Vector<vk::DescriptorImageInfo>  imageInfos;
        bufferInfos.Reserve(desc.Entries.Size());
        imageInfos.Reserve(desc.Entries.Size());
        Vector<vk::WriteDescriptorSet> writes;

        for (const auto& entry : desc.Entries)
        {
            const BindingLayoutEntry* binding
                = desc.Layout->Desc().Find(entry.Binding);
            VtCoreAssertFormat(binding,
                               "Vulkan: BindGroup '{}': binding {} isn't in "
                               "the layout",
                               desc.DebugName, entry.Binding);
            VtCoreAssertFormat(entry.ArrayElement < binding->Count,
                               "Vulkan: BindGroup '{}': element {} of binding "
                               "{} is out of range",
                               desc.DebugName, entry.ArrayElement,
                               entry.Binding);

            vk::WriteDescriptorSet& write = writes.EmplaceBack();
            write.dstSet                  = m_Set;
            write.dstBinding              = entry.Binding;
            write.dstArrayElement         = entry.ArrayElement;
            write.descriptorCount         = 1;
            write.descriptorType          = ToVk(binding->Type);

            if (IsBufferBinding(binding->Type))
            {
                VtCoreAssertFormat(entry.Buffer,
                                   "Vulkan: BindGroup '{}': binding {} needs "
                                   "a buffer",
                                   desc.DebugName, entry.Binding);
                auto* buffer = static_cast<VulkanBuffer*>(entry.Buffer.Raw());
                vk::DescriptorBufferInfo& info = bufferInfos.EmplaceBack();
                info.buffer = buffer->Handle();
                info.offset = entry.Offset;
                info.range  = entry.Range ? entry.Range : VK_WHOLE_SIZE;
                write.pBufferInfo = &info;
                continue;
            }

            vk::DescriptorImageInfo& info = imageInfos.EmplaceBack();
            if (binding->Type != BindingType::eSampler)
            {
                VtCoreAssertFormat(entry.Texture,
                                   "Vulkan: BindGroup '{}': binding {} needs "
                                   "a texture",
                                   desc.DebugName, entry.Binding);
                auto* texture = static_cast<VulkanTexture*>(entry.Texture.Raw());
                info.imageView   = binding->Type == BindingType::eStorageTexture
                                     ? texture->StorageView(entry.MipLevel)
                                     : texture->View();
                info.imageLayout = binding->Type == BindingType::eStorageTexture
                                     ? vk::ImageLayout::eGeneral
                                     : vk::ImageLayout::eShaderReadOnlyOptimal;
            }
            if (binding->Type == BindingType::eSampledTexture
                || binding->Type == BindingType::eSampler)
            {
                VtCoreAssertFormat(entry.Sampler,
                                   "Vulkan: BindGroup '{}': binding {} needs "
                                   "a sampler",
                                   desc.DebugName, entry.Binding);
                info.sampler
                    = static_cast<VulkanSampler*>(entry.Sampler.Raw())->Handle();
            }
            write.pImageInfo = &info;
        }

        if (!writes.Empty())
            device.Handle().updateDescriptorSets(writes.Size(), writes.Raw(), 0,
                                                 nullptr);
    }
    VulkanBindGroup::~VulkanBindGroup()
    {
        auto& device = VulkanDevice::Get();
        device.Defer(Deletion::eDescriptorSet,
                     reinterpret_cast<u64>(static_cast<VkDescriptorSet>(m_Set)),
                     VK_NULL_HANDLE,
                     reinterpret_cast<u64>(static_cast<VkDescriptorPool>(m_Pool)));
    }

    //------------------------------------------------------------------------
    // Pipelines -------------------------------------------------------------
    //------------------------------------------------------------------------

    VulkanPipeline::VulkanPipeline(const GraphicsPipelineDesc& desc)
        : Pipeline(desc)
    {
        VtCoreAssertMsg(desc.Shader, "Vulkan: Pipeline without a shader");
        VtCoreAssertMsg(!desc.ColorFormats.Empty()
                            || desc.DepthFormat != Format::eUndefined,
                        "Vulkan: Pipeline without attachments");
        auto& device = VulkanDevice::Get();
        auto* shader = static_cast<VulkanShader*>(desc.Shader.Raw());

        CreateLayout(*shader);

        Vector<vk::PipelineShaderStageCreateInfo> stages;
        for (const auto& module : shader->Modules())
        {
            vk::PipelineShaderStageCreateInfo& stage = stages.EmplaceBack();
            stage.stage                              = ToVkStage(module.Stage);
            stage.module                             = module.Handle;
            stage.pName                              = "main";
        }

        Vector<vk::VertexInputBindingDescription>   bindings;
        Vector<vk::VertexInputAttributeDescription> attributes;
        for (u32 slot = 0; slot < desc.VertexBindings.Size(); ++slot)
        {
            const auto& binding = desc.VertexBindings[slot];
            bindings.PushBack(vk::VertexInputBindingDescription(
                slot, binding.Stride,
                binding.PerInstance ? vk::VertexInputRate::eInstance
                                    : vk::VertexInputRate::eVertex));
            for (const auto& attribute : binding.Attributes)
                attributes.PushBack(vk::VertexInputAttributeDescription(
                    attribute.Location, slot, ToVk(attribute.Format),
                    attribute.Offset));
        }
        vk::PipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.vertexBindingDescriptionCount   = bindings.Size();
        vertexInput.pVertexBindingDescriptions      = bindings.Raw();
        vertexInput.vertexAttributeDescriptionCount = attributes.Size();
        vertexInput.pVertexAttributeDescriptions    = attributes.Raw();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.topology = ToVk(desc.Topology);

        vk::PipelineViewportStateCreateInfo viewport{};
        viewport.viewportCount = 1;
        viewport.scissorCount  = 1;

        vk::PipelineRasterizationStateCreateInfo raster{};
        raster.depthClampEnable = desc.Raster.DepthClamp;
        raster.polygonMode      = ToVk(desc.Raster.Polygon);
        raster.cullMode         = ToVk(desc.Raster.Cull);
        raster.frontFace        = ToVk(desc.Raster.Front);
        raster.lineWidth        = desc.Raster.LineWidth;

        vk::PipelineMultisampleStateCreateInfo multisample{};
        multisample.rasterizationSamples
            = static_cast<vk::SampleCountFlagBits>(desc.Samples);

        vk::PipelineDepthStencilStateCreateInfo depth{};
        bool hasDepth          = desc.DepthFormat != Format::eUndefined;
        depth.depthTestEnable  = hasDepth && desc.DepthStencil.DepthTest;
        depth.depthWriteEnable = hasDepth && desc.DepthStencil.DepthWrite;
        depth.depthCompareOp   = ToVk(desc.DepthStencil.DepthCompare);

        Vector<vk::PipelineColorBlendAttachmentState> blends;
        for (u32 i = 0; i < desc.ColorFormats.Size(); ++i)
        {
            BlendAttachment blend = i < desc.Blend.Size() ? desc.Blend[i]
                                                          : BlendAttachment{};
            vk::PipelineColorBlendAttachmentState& state = blends.EmplaceBack();
            state.blendEnable         = blend.Enable;
            state.srcColorBlendFactor = ToVk(blend.SrcColor);
            state.dstColorBlendFactor = ToVk(blend.DstColor);
            state.colorBlendOp        = ToVk(blend.ColorOp);
            state.srcAlphaBlendFactor = ToVk(blend.SrcAlpha);
            state.dstAlphaBlendFactor = ToVk(blend.DstAlpha);
            state.alphaBlendOp        = ToVk(blend.AlphaOp);
            state.colorWriteMask      = ToVk(blend.WriteMask);
        }
        vk::PipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.attachmentCount = blends.Size();
        colorBlend.pAttachments    = blends.Raw();

        vk::DynamicState dynamicStates[]
            = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamic{};
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates    = dynamicStates;

        // Dynamic rendering: attachment formats instead of a render pass.
        Vector<vk::Format> colorFormats;
        for (Format format : desc.ColorFormats) colorFormats.PushBack(ToVk(format));
        vk::PipelineRenderingCreateInfo rendering{};
        rendering.colorAttachmentCount    = colorFormats.Size();
        rendering.pColorAttachmentFormats = colorFormats.Raw();
        rendering.depthAttachmentFormat   = ToVk(desc.DepthFormat);
        if (HasStencil(desc.DepthFormat))
            rendering.stencilAttachmentFormat = ToVk(desc.DepthFormat);

        vk::GraphicsPipelineCreateInfo info{};
        info.pNext               = &rendering;
        info.stageCount          = stages.Size();
        info.pStages             = stages.Raw();
        info.pVertexInputState   = &vertexInput;
        info.pInputAssemblyState = &inputAssembly;
        info.pViewportState      = &viewport;
        info.pRasterizationState = &raster;
        info.pMultisampleState   = &multisample;
        info.pDepthStencilState  = &depth;
        info.pColorBlendState    = &colorBlend;
        info.pDynamicState       = &dynamic;
        info.layout              = m_Layout;

        VkCall(device.Handle().createGraphicsPipelines(device.PipelineCache(), 1, &info,
                                                       nullptr, &m_Pipeline));
        SetDebugName(device.Handle(), m_Pipeline, desc.DebugName);
    }
    void VulkanPipeline::CreateLayout(VulkanShader& shader)
    {
        auto& device = VulkanDevice::Get();
        // Layout: the shader's set layouts and push constant range.
        Vector<vk::PushConstantRange> pushConstants;
        for (const auto& range : shader.Layout().PushConstants)
        {
            vk::PushConstantRange& vkRange = pushConstants.EmplaceBack();
            vkRange.stageFlags             = ToVk(range.Stages);
            vkRange.offset                 = range.Offset;
            vkRange.size                   = range.Size;
            m_PushConstantStages |= vkRange.stageFlags;
        }
        const auto&                  setLayouts = shader.SetLayoutHandles();
        vk::PipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.setLayoutCount         = setLayouts.Size();
        layoutInfo.pSetLayouts            = setLayouts.Raw();
        layoutInfo.pushConstantRangeCount = pushConstants.Size();
        layoutInfo.pPushConstantRanges    = pushConstants.Raw();
        VkCall(device.Handle().createPipelineLayout(&layoutInfo, nullptr,
                                                    &m_Layout));
    }

    VulkanPipeline::VulkanPipeline(const ComputePipelineDesc& desc)
        : Pipeline(desc)
    {
        VtCoreAssertMsg(desc.Shader, "Vulkan: Compute pipeline without a shader");
        auto& device = VulkanDevice::Get();
        auto* shader = static_cast<VulkanShader*>(desc.Shader.Raw());
        VtCoreAssertMsg(shader->Stages() == ShaderStage::eCompute,
                        "Vulkan: A compute pipeline needs a compute-only shader");
        CreateLayout(*shader);

        vk::ComputePipelineCreateInfo info{};
        info.stage.stage  = vk::ShaderStageFlagBits::eCompute;
        info.stage.module = shader->Modules()[0].Handle;
        info.stage.pName  = "main";
        info.layout       = m_Layout;
        VkCall(device.Handle().createComputePipelines(device.PipelineCache(), 1, &info,
                                                      nullptr, &m_Pipeline));
        SetDebugName(device.Handle(), m_Pipeline, desc.DebugName);
    }

    VulkanPipeline::~VulkanPipeline()
    {
        auto& device = VulkanDevice::Get();
        device.Defer(Deletion::ePipeline, m_Pipeline);
        device.Defer(Deletion::ePipelineLayout, m_Layout);
    }
} // namespace Vortex::Vulkan
