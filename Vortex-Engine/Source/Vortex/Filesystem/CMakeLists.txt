#
# Created by v1tr10l7 on 13.09.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Filesystem)
set (MODULE_TYPE SHARED)

set (SOURCES
    Filesystem.cpp
)

vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS Vortex-Modules Vortex-Core
)
