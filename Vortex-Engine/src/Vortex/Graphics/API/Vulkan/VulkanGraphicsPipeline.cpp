//
// Created by Vitriol1744 on 15.09.2022.
//
#include "VulkanGraphicsPipeline.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanContext.hpp"

namespace Vortex::Graphics
{
    static VkFormat ToVkFormat(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::eBool:
                return VK_FORMAT_R8_SINT;
                break;
            case ShaderDataType::eInt:
                return VK_FORMAT_R32_SINT;
                break;
            case ShaderDataType::eInt2:
                return VK_FORMAT_R32G32_SINT;
                break;
            case ShaderDataType::eInt3:
                return VK_FORMAT_R32G32B32_SINT;
                break;
            case ShaderDataType::eInt4:
                return VK_FORMAT_R32G32B32A32_SINT;
                break;
            case ShaderDataType::eFloat:
                return VK_FORMAT_R32_SFLOAT;
                break;
            case ShaderDataType::eFloat2:
                return VK_FORMAT_R32G32_SFLOAT;
                break;
            case ShaderDataType::eFloat3:
                return VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case ShaderDataType::eFloat4:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            case ShaderDataType::eMat3:
                break;
            case ShaderDataType::eMat4:
                break;
        }

        return VK_FORMAT_UNDEFINED;
    }

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(GraphicsPipelineCreateInfo& createInfo, Ref<IWindow> window)
    {
        context = dynamic_cast<VulkanContext*>(window->GetGraphicsContext());
        context->AttachGraphicsPipeline(this, createInfo);
        bound = this;
        context->SetGraphicsPipeline(this);
    }
    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        context->DetachGraphicsPipeline(this);
        bound = nullptr;
    }

    void VulkanGraphicsPipeline::Initialize(GraphicsPipelineCreateInfo& createInfo, VkExtent2D extent, VkFormat format)
    {
        Ref<IShader> shader = createInfo.shader;
        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.pNext = VK_NULL_HANDLE;
        dynamicStateCreateInfo.flags = 0;
        dynamicStateCreateInfo.dynamicStateCount = std::size(dynamicStates);
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;

        VertexBufferLayout& layout = createInfo.layout;
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = layout.GetStride();
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(layout.GetElementCount());
        VkVertexInputAttributeDescription attributeDescription;
        uint32 index = 0;
        for (auto& element : layout)
        {
            attributeDescription.binding = 0;
            attributeDescription.location = index++;
            attributeDescription.format = ToVkFormat(element.type);
            attributeDescription.offset = element.offset;
            attributeDescriptions.push_back(attributeDescription);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.pNext = VK_NULL_HANDLE;
        vertexInputCreateInfo.flags = 0;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size());
        vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.pNext = VK_NULL_HANDLE;
        inputAssemblyCreateInfo.flags = 0;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = extent.width;
        viewport.height = extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = extent.width;
        scissor.extent.height = extent.height;

        VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
        viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportCreateInfo.pNext = VK_NULL_HANDLE;
        viewportCreateInfo.flags = 0;
        viewportCreateInfo.viewportCount = 1;
        viewportCreateInfo.pViewports = &viewport;
        viewportCreateInfo.scissorCount = 1;
        viewportCreateInfo.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.pNext = VK_NULL_HANDLE;
        rasterizationStateCreateInfo.flags = 0;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.pNext = VK_NULL_HANDLE;
        multisampleStateCreateInfo.flags = 0;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.minSampleShading = 1.0f;
        multisampleStateCreateInfo.pSampleMask = VK_NULL_HANDLE;
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext = VK_NULL_HANDLE;
        colorBlendStateCreateInfo.flags = 0;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;
        CreateRenderPass(format);

        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        //TODO: Support for other types of shaders
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        VkCall_msg(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout),
                   "Failed to create descriptor set layout!");

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        Ref<VulkanShader> vkShader = std::dynamic_pointer_cast<VulkanShader>(shader);
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.pNext = VK_NULL_HANDLE;
        pipelineCreateInfo.flags = 0;
        pipelineCreateInfo.stageCount = vkShader->GetShaderStages().size();
        pipelineCreateInfo.pStages = vkShader->GetShaderStages().data();
        pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pTessellationState = VK_NULL_HANDLE;
        pipelineCreateInfo.pViewportState = &viewportCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = VK_NULL_HANDLE;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

        VkCall_msg(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, allocator, &graphicsPipeline),
                   "Failed to Create Graphics Pipeline!");
    }
    void VulkanGraphicsPipeline::Destroy()
    {
        if (graphicsPipeline == VK_NULL_HANDLE) return;
        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        graphicsPipeline = VK_NULL_HANDLE;
    }

    void VulkanGraphicsPipeline::Bind()
    {
        bound = this;
        context->SetGraphicsPipeline(this);
    }

    void VulkanGraphicsPipeline::CreateRenderPass(VkFormat format)
    {
        colorAttachment.format = format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachment;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        VkDevice device = VulkanRendererAPI::GetDevice().GetLogicalDevice();
        VkAllocationCallbacks* allocator = VulkanAllocator::Get().callbacks;

        VkCall_msg(vkCreateRenderPass(device, &renderPassCreateInfo, allocator, &renderPass),
                   "Failed to Create RenderPass!");
    }
}