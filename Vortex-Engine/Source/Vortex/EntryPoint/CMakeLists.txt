#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-EntryPoint)
set (MODULE_TYPE STATIC)

set (SOURCES
    EntryPoint.cpp
)
set (PRIVATE_INCLUDES)
if(ANDROID)
    list(APPEND PRIVATE_INCLUDES ${VENDOR_DIR}/android_native_app_glue)
endif()


vt_add_module(${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE ${MODULE_TYPE}
    PUBLIC_LIBS Vortex-Modules Vortex-Core 
    PRIVATE_INCLUDES ${PRIVATE_INCLUDES}
)
