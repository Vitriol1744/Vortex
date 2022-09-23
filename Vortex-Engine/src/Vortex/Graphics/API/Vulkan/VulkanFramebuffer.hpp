//
// Created by Vitriol1744 on 15.09.2022.
//
#ifndef VORTEX_VULKANFRAMEBUFFER_HPP
#define VORTEX_VULKANFRAMEBUFFER_HPP

#include "Vortex/Graphics/API/Vulkan/VulkanCommon.hpp"

#include "Vortex/Graphics/API/IFramebuffer.hpp"

namespace Vortex::Graphics
{
    class VulkanContext;

    class VT_API VulkanFramebuffer : public IFramebuffer
    {
        public:
            VulkanFramebuffer(Ref<IGraphicsPipeline> pipeline);
            ~VulkanFramebuffer();

            void Initialize(std::vector<VkImageView> imageViews, VkRenderPass renderPass, VkExtent2D extent);
            void Destroy();

            virtual void Bind() override;

            inline std::vector<VkFramebuffer>& GetFramebuffers() noexcept { return framebuffers; }

        private:
            VulkanContext* context = nullptr;
            std::vector<VkFramebuffer> framebuffers;
    };
}
#endif //VORTEX_VULKANFRAMEBUFFER_HPP
