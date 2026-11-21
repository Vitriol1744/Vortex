#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#

# The backend-agnostic RHI: interface modules (umbrella: Vortex.RHI) and the
# device registry. Shared, so the global device and the interfaces' vtables
# exist once even though several shared libraries use them.
vt_add_module(Vortex-RHI
    SOURCES
        Device.cpp
        PipelineCache.cpp
    TYPE SHARED
    PUBLIC_LIBS
        Vortex-Modules
        Vortex-Core
)
target_sources(Vortex-RHI PUBLIC FILE_SET CXX_MODULES FILES
    BindGroup.ixx
    CommandList.ixx
    Device.ixx
    Pipeline.ixx
    PipelineCache.ixx
    Resources.ixx
    RHI.ixx
    Shader.ixx
    SwapChain.ixx
    Types.ixx
)

add_subdirectory(Vulkan)
