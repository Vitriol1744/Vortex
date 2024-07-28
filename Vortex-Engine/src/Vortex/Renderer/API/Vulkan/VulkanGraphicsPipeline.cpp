/*
 * Created by v1tr10l7 on 28.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Renderer/API/Vulkan/VulkanContext.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanGraphicsPipeline.hpp"
#include "Vortex/Renderer/API/Vulkan/VulkanShader.hpp"

namespace Vortex
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        const GraphicsPipelineSpecification& specification)
    {
        auto shader
            = std::dynamic_pointer_cast<VulkanShader>(specification.Shader);

        vk::PipelineShaderStageCreateInfo vertexShaderStage{};
        vertexShaderStage.sType
            = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertexShaderStage.stage  = vk::ShaderStageFlagBits::eVertex;
        vertexShaderStage.module = shader->GetVertexShader();
        vertexShaderStage.pName  = "main";

        vk::PipelineShaderStageCreateInfo fragmentShaderStage{};
        fragmentShaderStage.sType
            = vk::StructureType::ePipelineShaderStageCreateInfo;
        fragmentShaderStage.stage  = vk::ShaderStageFlagBits::eFragment;
        fragmentShaderStage.module = shader->GetFragmentShader();
        fragmentShaderStage.pName  = "main";

        vk::PipelineShaderStageCreateInfo shaderStages[]
            = {vertexShaderStage, fragmentShaderStage};

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType
            = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertexInputInfo.vertexBindingDescriptionCount   = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

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
        rasterizer.frontFace               = vk::FrontFace::eClockwise;
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
        colorBlendAttachment.blendEnable = VK_FALSE;

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
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        auto context = std::dynamic_pointer_cast<VulkanContext>(
            specification.Window->GetRendererContext());
        vk::Device device = VulkanContext::GetDevice();

        VkCall(device.createPipelineLayout(&pipelineLayoutInfo, VK_NULL_HANDLE,
                                           &m_PipelineLayout));

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
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
        vk::Device device = VulkanContext::GetDevice();

        device.destroyPipeline(m_Pipeline, VK_NULL_HANDLE);
        device.destroyPipelineLayout(m_PipelineLayout, VK_NULL_HANDLE);
    }
}; // namespace Vortex
