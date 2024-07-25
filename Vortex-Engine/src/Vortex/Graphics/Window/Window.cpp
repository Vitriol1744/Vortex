/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include "vtpch.hpp"

#include "Vortex/Core/Platform.hpp"
#include "Vortex/Graphics/Window/Window.hpp"

#ifdef VT_PLATFORM_LINUX
    #include "Vortex/Graphics/Window/X11/X11Window.hpp"
#endif

namespace Vortex
{
    void Window::PollEvents()
    {
#ifdef VT_PLATFORM_LINUX
        X11Window::PollEvents();
#else
    #error "Polling events is not supported by Vortex on this platform."
#endif
    }

    Ref<Window> Window::Create(const WindowSpecification& specification)
    {
#ifdef VT_PLATFORM_LINUX
        return CreateRef<X11Window>(specification);
#else
    #error "Window creation is not supported by Vortex on this platform."
#endif
    }

}; // namespace Vortex
