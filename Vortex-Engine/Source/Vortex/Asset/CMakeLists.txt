#
# Created by v1tr10l7 on 11.07.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Asset)
set (MODULE_TYPE SHARED)

set (SOURCES
    AssetManager.cpp
    AssetRegistry.cpp
    AsyncAssetLoader.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS Vortex-Modules Vortex-Core 
)
target_sources(${MODULE_NAME} PUBLIC FILE_SET CXX_MODULES FILES
    AsyncAssetLoader.ixx
)
if(TARGET Tracy::TracyClient)
  target_link_libraries(${MODULE_NAME} PUBLIC Tracy::TracyClient)
endif()
