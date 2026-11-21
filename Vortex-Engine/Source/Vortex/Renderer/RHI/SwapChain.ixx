/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Core/Types.hpp>
#include <Vortex/Renderer/RHI/Export.hpp>

// In the global module fragment, so it names the Window class of
// <Vortex/Window/Window.hpp> instead of attaching a new one to this module.
namespace Vortex
{
    class Window;
}

export module Vortex.RHI.SwapChain;

export import Vortex.RHI.Resources;

import Prism.Memory.Ref;
import Prism.Memory.RefCounted;

/**
 * The images a window presents. Per frame:
 *
 *   if (!swapChain->AcquireNextImage()) skip;      // minimized / recreating
 *   auto image = swapChain->CurrentImage();         // render into it
 *   device.Submit(cmd, {&swapChain, 1});            // waits, then presents
 *
 * Out-of-date and suboptimal swapchains are recreated transparently on the
 * next acquire; so are size and vsync changes.
 */
export namespace Vortex::RHI
{
    struct SwapChainDesc
    {
        class Window* Window          = nullptr;
        bool          VSync           = true;
        /// Used if the surface supports it, otherwise the closest match.
        RHI::Format   PreferredFormat = RHI::Format::eBGRA8Unorm;
    };

    class VT_RENDERER_RHI_API SwapChain : public RefCounted<SwapChain>
    {
      public:
        SwapChain(const SwapChain&)            = delete;
        SwapChain& operator=(const SwapChain&) = delete;
        virtual ~SwapChain()                   = default;

        /// Acquires the image to render into this frame. Returns false when
        /// there is nothing to present to (zero-sized window); skip
        /// rendering to this swapchain for the frame then.
        virtual bool                AcquireNextImage() = 0;
        /// Image acquired by the last successful AcquireNextImage.
        virtual ::Ref<RHI::Texture> CurrentImage() const = 0;

        virtual RHI::Format         Format() const = 0;
        virtual Extent2D            Extent() const = 0;
        virtual u32                 ImageCount() const = 0;

        /// Applied when the next image is acquired.
        virtual void                SetVSync(bool vsync) = 0;
        virtual bool                VSync() const = 0;
        /// Forces recreation on the next acquire (e.g. after a resize event).
        virtual void                RequestResize() = 0;

        inline class Window*        Window() const { return m_Window; }

      protected:
        explicit SwapChain(class Window* window)
            : m_Window(window)
        {
        }

        class Window* m_Window = nullptr;
    };
} // namespace Vortex::RHI
