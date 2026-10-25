#
# Created by v1tr10l7 on 11.07.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Script)
set (MODULE_TYPE STATIC)

set (SOURCES
  ScriptEngine.cpp
  Glue/ScriptGlue.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS Vortex-Modules Vortex-Core Coral.Native Vortex-RHI Vortex-Scene
)
