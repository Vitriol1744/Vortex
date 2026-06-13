/*
 * Created by v1tr10l7 on 08.09.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Vortex/Core/Core.hpp>
#include <Vortex/Core/Math/Vector.hpp>
#include <Vortex/Renderer/API/Image.hpp>

namespace Vortex
{
    enum class ImageFormat : u32
    {
        eUndefined,
        eRGB,
        eRGBA,
        eDepth24Stencil8,
        eRedInteger,
    };

    struct FramebufferAttachmentSpecification
    {
    };

    struct FramebufferSpecification
    {
        bool                             SwapChainTarget = false;
        u32                              Width           = 0;
        u32                              Height          = 0;

        Vector<FramebufferSpecification> Attachments{};
    };

    class VT_API Framebuffer : public NonCopyable<Framebuffer>,
                               public RefCounted
    {
      public:
        virtual ~Framebuffer()                                 = default;

        virtual Math::Vec2u    Size() const                    = 0;
        virtual ::Ref<Image2D> Image(u32 index = 0) const      = 0;

        virtual void           OnResize(u32 width, u32 height) = 0;

        static ::Ref<Framebuffer>
        Create(const FramebufferSpecification& specification);
    };
}; // namespace Vortex
