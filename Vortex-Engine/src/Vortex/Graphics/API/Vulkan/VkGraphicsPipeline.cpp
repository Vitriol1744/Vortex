//
// Created by Vitriol1744 on 04.09.2021.
//
#include "VkGraphicsPipeline.hpp"

#include "Vortex/Graphics/API/Vulkan/VkRendererAPI.hpp"

namespace Vortex::Graphics
{
    VkGraphicsPipeline::~VkGraphicsPipeline()
    {
        vk::Device device = VkRendererAPI::GetDevice().GetLogicalDevice();

        device.destroyPipeline(graphicsPipeline, VkAllocator::Get().callbacks);
        device.destroyPipelineLayout(pipelineLayout, VkAllocator::Get().callbacks);
    }

    void VkGraphicsPipeline::Initialize(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader)
    {
        vk::Device device = VkRendererAPI::GetDevice().GetLogicalDevice();

        vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
        vertexShaderStageCreateInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertexShaderStageCreateInfo.pNext = VK_NULL_HANDLE;
        vertexShaderStageCreateInfo.flags = vk::PipelineShaderStageCreateFlagBits();
        vertexShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertexShaderStageCreateInfo.module = vertexShader;
        vertexShaderStageCreateInfo.pName = "main";
        vertexShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;

        vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
        vertexShaderStageCreateInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        vertexShaderStageCreateInfo.pNext = VK_NULL_HANDLE;
        vertexShaderStageCreateInfo.flags = vk::PipelineShaderStageCreateFlagBits();
        vertexShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
        vertexShaderStageCreateInfo.module = fragmentShader;
        vertexShaderStageCreateInfo.pName = "main";
        vertexShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;

        vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
        vertexInputCreateInfo.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertexInputCreateInfo.pNext = VK_NULL_HANDLE;
        vertexInputCreateInfo.flags = vk::PipelineVertexInputStateCreateFlagBits();
        vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputCreateInfo.pVertexBindingDescriptions = VK_NULL_HANDLE;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputCreateInfo.pVertexAttributeDescriptions = VK_NULL_HANDLE;

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
        inputAssemblyCreateInfo.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        inputAssemblyCreateInfo.pNext = VK_NULL_HANDLE;
        inputAssemblyCreateInfo.flags = vk::PipelineInputAssemblyStateCreateFlagBits();
        inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        vk::Viewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 800; //TODO: Window Bounds instead of hardcoding viewport
        viewport.height = 600;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = 800; //TODO: Window Bounds instead of hardcoding scissor
        scissor.extent.height = 600;

        vk::PipelineViewportStateCreateInfo viewportCreateInfo = {};
        viewportCreateInfo.sType = vk::StructureType::ePipelineViewportStateCreateInfo;
        viewportCreateInfo.pNext = VK_NULL_HANDLE;
        viewportCreateInfo.flags = vk::PipelineViewportStateCreateFlagBits();
        viewportCreateInfo.viewportCount = 1;
        viewportCreateInfo.pViewports = &viewport;
        viewportCreateInfo.scissorCount = 1;
        viewportCreateInfo.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
        rasterizationStateCreateInfo.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        rasterizationStateCreateInfo.pNext = VK_NULL_HANDLE;
        rasterizationStateCreateInfo.flags = vk::PipelineRasterizationStateCreateFlagBits();
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
        rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
        rasterizationStateCreateInfo.frontFace = vk::FrontFace::eClockwise;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        multisampleStateCreateInfo.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        multisampleStateCreateInfo.pNext = VK_NULL_HANDLE;
        multisampleStateCreateInfo.flags = vk::PipelineMultisampleStateCreateFlagBits();
        multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.minSampleShading = 1.0f;
        multisampleStateCreateInfo.pSampleMask = VK_NULL_HANDLE;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.colorWriteMask =
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
        colorBlendStateCreateInfo.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        colorBlendStateCreateInfo.pNext = VK_NULL_HANDLE;
        colorBlendStateCreateInfo.flags = vk::PipelineColorBlendStateCreateFlagBits();
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };
        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        dynamicStateCreateInfo.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        dynamicStateCreateInfo.pNext = VK_NULL_HANDLE;
        dynamicStateCreateInfo.flags = vk::PipelineDynamicStateCreateFlagBits();
        dynamicStateCreateInfo.dynamicStateCount = std::size(dynamicStates);
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.pNext = VK_NULL_HANDLE;
        pipelineLayoutCreateInfo.flags = vk::PipelineLayoutCreateFlagBits();
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pSetLayouts = VK_NULL_HANDLE;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;

        VkCall(device.createPipelineLayout(&pipelineLayoutCreateInfo, VkAllocator::Get().callbacks, &pipelineLayout),
            "Failed to Create Pipeline Layout!");

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipelineCreateInfo.pNext = VK_NULL_HANDLE;
        pipelineCreateInfo.flags = vk::PipelineCreateFlagBits();
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shaderStages;
        pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pTessellationState = VK_NULL_HANDLE;
        pipelineCreateInfo.pViewportState = &viewportCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = VK_NULL_HANDLE;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineCreateInfo.pDynamicState = VK_NULL_HANDLE;
        pipelineCreateInfo.layout = pipelineLayout;
        //pipelineCreateInfo.renderPass = ;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

        VkCall(device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, VkAllocator::Get().callbacks, &graphicsPipeline),
               "Failed to Create Graphics Pipeline!");
    }
}