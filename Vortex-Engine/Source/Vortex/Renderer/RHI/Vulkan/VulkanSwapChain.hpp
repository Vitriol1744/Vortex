/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Renderer/RHI/Vulkan/VulkanCommon.hpp>
#include <Vortex/Renderer/RHI/Vulkan/VulkanSurface.hpp>

#include <atomic>

import Prism.Containers.Vector;
import Prism.Memory.Ref;

namespace Vortex::Vulkan
{
    class VulkanTexture;

    class VulkanSwapChain final : public RHI::SwapChain
    {
      public:
        explicit VulkanSwapChain(const RHI::SwapChainDesc& desc);
        ~VulkanSwapChain() override;

        /// False if the surface couldn't be created; the swapchain is then
        /// unusable and the device returns null instead.
        inline bool                 Valid() const { return m_Valid; }

        bool                        AcquireNextImage() override;
        ::Ref<RHI::Texture>         CurrentImage() const override;
        RHI::Format                 Format() const override { return m_Format; }
        RHI::Extent2D               Extent() const override { return m_Extent; }
        u32                         ImageCount() const override
        {
            return m_Images.Size();
        }
        void                        SetVSync(bool vsync) override;
        bool                        VSync() const override { return m_VSync; }
        void                        RequestResize() override
        {
            m_NeedsRecreate = true;
        }

        //--------------------------------------------------------------------
        // Used by VulkanDevice::Submit --------------------------------------
        //--------------------------------------------------------------------

        /// Whether an image was acquired and not yet presented.
        inline bool          HasAcquiredImage() const { return m_Acquired; }
        /// Signaled by the acquire; the submit waits on it.
        inline vk::Semaphore AcquireSemaphore() const
        {
            return m_AcquireSemaphores[m_AcquireIndex];
        }
        /// Signaled by the submit; presentation waits on it.
        inline vk::Semaphore PresentSemaphore() const
        {
            return m_PresentSemaphores[m_ImageIndex];
        }
        inline vk::SwapchainKHR Handle() const { return m_SwapChain; }
        inline u32              ImageIndex() const { return m_ImageIndex; }
        /// Called after the presenting submit with the vkQueuePresentKHR
        /// result.
        void                    OnPresented(vk::Result result);

      private:
        VulkanSurface                m_Surface;
        vk::SwapchainKHR             m_SwapChain = VK_NULL_HANDLE;
        Vector<::Ref<VulkanTexture>> m_Images;
        /// One per frame in flight: the acquire of frame N reuses the
        /// semaphore of frame N - FramesInFlight, whose fence has passed.
        Vector<vk::Semaphore>        m_AcquireSemaphores;
        /// One per image: an image's semaphore is free again once the image
        /// is re-acquired, which is the only reuse rule presentation allows.
        Vector<vk::Semaphore>        m_PresentSemaphores;

        RHI::Format                  m_Format        = RHI::Format::eUndefined;
        vk::ColorSpaceKHR            m_ColorSpace{};
        RHI::Extent2D                m_Extent        = {};
        u32                          m_ImageIndex    = 0;
        u32                          m_AcquireIndex  = 0;
        bool                         m_VSync         = true;
        bool                         m_Acquired      = false;
        /// Set from any thread (resize events), consumed by the render thread.
        std::atomic<bool>            m_NeedsRecreate = false;
        bool                         m_Valid         = false;
        RHI::Format                  m_PreferredFormat;

        /// Creates (or recreates) the swapchain. Returns false if the window
        /// has no area to present to.
        bool                         Recreate();
        void                         ChooseFormat();
        vk::PresentModeKHR           ChoosePresentMode() const;
        void                         DestroySemaphores(Vector<vk::Semaphore>& semaphores);
    };
} // namespace Vortex::Vulkan
