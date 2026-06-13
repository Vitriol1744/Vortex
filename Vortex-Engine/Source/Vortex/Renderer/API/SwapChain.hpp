/*
 * Created by v1tr10l7 on 19.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/WeakRef.hpp>
#include <Vortex/Core/Core.hpp>

#include <Vortex/Renderer/API/Framebuffer.hpp>

namespace Vortex
{
    class SwapChain : public NonCopyable<SwapChain>, public RefCounted
    {
      public:
        SwapChain()                                             = default;
        virtual ~SwapChain()                                    = default;

        virtual WeakRef<class Framebuffer> Framebuffer()        = 0;

        virtual void                       Present()            = 0;
        virtual void                       OnResize()           = 0;

        virtual void                       SetVSync(bool vsync) = 0;

        static ::Ref<SwapChain>            Create(class Window* window,
                                                  bool          vsync = false);
    };
}; // namespace Vortex
