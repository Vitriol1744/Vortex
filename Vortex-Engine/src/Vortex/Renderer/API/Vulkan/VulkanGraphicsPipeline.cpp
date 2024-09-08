/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanRenderer.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

namespace Vortex
{
    static vk::Format ToVulkanFormat(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::eInt: return vk::Format::eR32Sint;
            case ShaderDataType::eInt2: return vk::Format::eR32G32Sint;
            case ShaderDataType::eInt3: return vk::Format::eR32G32B32Sint;
            case ShaderDataType::eInt4: return vk::Format::eR32G32B32A32Sint;
            case ShaderDataType::eFloat: return vk::Format::eR32Sfloat;
            case ShaderDataType::eFloat2: return vk::Format::eR32G32Sfloat;
            case ShaderDataType::eFloat3: return vk::Format::eR32G32B32Sfloat;
            case ShaderDataType::eFloat4:
                return vk::Format::eR32G32B32A32Sfloat;

            default: break;
        }

        VtCoreError("ToVulkanFormat: Undefined format");
        return vk::Format::eUndefined;
    }

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        const GraphicsPipelineSpecification& specification)
    {
        m_Shader
            = std::dynamic_pointer_cast<VulkanShader>(specification.Shader);

        vk::PipelineShaderStageCreateInfo vertexShaderStage{};
        vertexShaderStage.sType
            = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertexShaderStage.stage  = vk::ShaderStageFlagBits::eVertex;
        vertexShaderStage.module = m_Shader->GetVertexShader();
        vertexShaderStage.pName  = "main";

        vk::PipelineShaderStageCreateInfo fragmentShaderStage{};
        fragmentShaderStage.sType
            = vk::StructureType::ePipelineShaderStageCreateInfo;
        fragmentShaderStage.stage  = vk::ShaderStageFlagBits::eFragment;
        fragmentShaderStage.module = m_Shader->GetFragmentShader();
        fragmentShaderStage.pName  = "main";

        vk::PipelineShaderStageCreateInfo shaderStages[]
            = {vertexShaderStage, fragmentShaderStage};

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType
            = vk::StructureType::ePipelineVertexInputStateCreateInfo;

        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding   = 0;
        bindingDescription.stride    = specification.Layout.GetStride();
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.resize(specification.Layout.GetElementCount());
        u32 binding  = 0;
        u32 location = 0;
        for (const auto& element : specification.Layout)
        {
            attributeDescriptions[location].binding  = binding;
            attributeDescriptions[location].location = location;
            attributeDescriptions[location].format
                = ToVulkanFormat(element.Type);
            attributeDescriptions[location].offset = element.Offset;
            ++location;
        }

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount
            = attributeDescriptions.size();
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions
            = attributeDescriptions.data();

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType
            = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        vk::PipelineViewportStateCreateInfo viewportState{};
        viewportState.sType
            = vk::StructureType::ePipelineViewportStateCreateInfo;
        viewportState.viewportCount = 1;
        viewportState.scissorCount  = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType
            = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        rasterizer.depthClampEnable        = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = vk::PolygonMode::eFill;
        rasterizer.lineWidth               = 1.0f;
        rasterizer.cullMode                = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace               = vk::FrontFace::eCounterClockwise;
        rasterizer.depthBiasEnable         = VK_FALSE;

        vk::PipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType
            = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        multisampling.sampleShadingEnable  = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask
            = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
            | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable         = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor
            = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp        = vk::BlendOp::eAdd;
        colorBlendAttachment.alphaBlendOp        = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;

        vk::PipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType
            = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = vk::LogicOp::eCopy;
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<vk::DynamicState> dynamicStates
            = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        dynamicState.dynamicStateCount
            = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates      = dynamicStates.data();

        vk::Device  device               = VulkanRenderer::GetDevice();
        const auto& descriptorSetLayouts = m_Shader->GetDescriptorSetLayouts();

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        auto context = std::dynamic_pointer_cast<VulkanContext>(
            specification.Window->GetRendererContext());

        VkCall(device.createPipelineLayout(&pipelineLayoutInfo, VK_NULL_HANDLE,
                                           &m_PipelineLayout));

        vk::PipelineDepthStencilStateCreateInfo depthInfo{};
        depthInfo.sType
            = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
        depthInfo.depthTestEnable       = VK_TRUE;
        depthInfo.depthWriteEnable      = VK_TRUE;
        depthInfo.depthCompareOp        = vk::CompareOp::eLess;
        depthInfo.depthBoundsTestEnable = VK_FALSE;
        depthInfo.minDepthBounds        = 0.0f; // Optional
        depthInfo.maxDepthBounds        = 1.0f; // Optional
        depthInfo.stencilTestEnable     = VK_FALSE;

#if 0
        vk::PipelineRenderingCreateInfoKHR renderingInfo{};
        renderingInfo.sType = vk::StructureType::ePipelineRenderingCreateInfo;
        renderingInfo.colorAttachmentCount = 1;

        auto imageFormat = context->GetSwapChain().GetImageFormat();
        renderingInfo.pColorAttachmentFormats = &imageFormat;
#endif

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
#if 0
        pipelineInfo.pNext = &renderingInfo;
#endif
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pDepthStencilState  = &depthInfo;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout              = m_PipelineLayout;
        pipelineInfo.renderPass = context->GetSwapChain().GetRenderPass();
        pipelineInfo.subpass    = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VkCall(device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo,
                                              VK_NULL_HANDLE, &m_Pipeline));
    };
    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        vk::Device device = VulkanRenderer::GetDevice();

        device.destroyPipeline(m_Pipeline, VK_NULL_HANDLE);
        device.destroyPipelineLayout(m_PipelineLayout, VK_NULL_HANDLE);
    }
}; // namespace Vortex
