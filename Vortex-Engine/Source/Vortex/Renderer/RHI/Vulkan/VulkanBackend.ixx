/*
 * Created by v1tr10l7 on 15.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Vortex/Renderer/RHI/Vulkan/Export.hpp>

export module Vortex.RHI.Vulkan;

/**
 * The Vulkan backend's whole public surface: register it, then create the
 * device through RHI::Device::Create with API::eVulkan. Everything else of the
 * backend is private to Vortex-Vulkan, so no Vulkan type reaches engine code.
 */
export extern "C++"
{
    namespace Vortex::Vulkan
    {
        /// Registers the Vulkan backend with RHI::Device. Idempotent.
        VT_RENDERER_RHI_VULKAN_API void RegisterBackend();
    } // namespace Vortex::Vulkan
}
