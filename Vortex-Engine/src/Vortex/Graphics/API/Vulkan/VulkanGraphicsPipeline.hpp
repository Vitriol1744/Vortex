//
// Created by Vitriol1744 on 15.09.2022.
//
#ifndef VORTEX_VULKANGRAPHICSPIPELINE_HPP
#define VORTEX_VULKANGRAPHICSPIPELINE_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IGraphicsPipeline.hpp"

namespace Vortex::Graphics
{
    class VulkanContext;

    class VT_API VulkanGraphicsPipeline : public IGraphicsPipeline
    {
        public:
            VulkanGraphicsPipeline(GraphicsPipelineCreateInfo& createInfo, Ref<IWindow> window);
            ~VulkanGraphicsPipeline();

            void Initialize(GraphicsPipelineCreateInfo& createInfo, VkExtent2D extent, VkFormat format);
            void Destroy();

            virtual void Bind() override;

            inline static VulkanGraphicsPipeline* GetBound() noexcept { return bound; }
            inline VulkanContext* GetContext() noexcept { return context; }

            inline VkPipeline& GetPipeline() noexcept { return graphicsPipeline; }
            inline VkRenderPass GetRenderPass() noexcept { return renderPass; }
            inline VkAttachmentDescription* GetAttachments() { return &colorAttachment; }

        private:
            static inline VulkanGraphicsPipeline* bound = nullptr;
            VulkanContext* context;

            VkPipeline graphicsPipeline = VK_NULL_HANDLE;
            VkPipelineLayout pipelineLayout;
            VkRenderPass renderPass;
            VkDescriptorSetLayout descriptorSetLayout;
            VkAttachmentDescription colorAttachment = {};

            void CreateRenderPass(VkFormat format);
    };
}
#endif //VORTEX_VULKANGRAPHICSPIPELINE_HPP
