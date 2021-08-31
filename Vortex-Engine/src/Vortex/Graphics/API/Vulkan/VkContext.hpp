//
// Created by vitriol1744 on 27.08.2021.
//
#pragma once

#include "Vortex/Core/Core.hpp"

#include "Vortex/Graphics/API/IGraphicsContext.hpp"

#include "Vortex/Graphics/API/Vulkan/VkInstance.hpp"
#include "Vortex/Graphics/API/Vulkan/VkSurface.hpp"

namespace Vortex::Graphics
{
    class VT_API VkContext : public IGraphicsContext
    {
        public:
            VkContext(NativeWindowHandleType window);
            ~VkContext();

            virtual void Present() override;
            virtual void Activate() override;

        private:
            VkSurface windowSurface;
    };
}