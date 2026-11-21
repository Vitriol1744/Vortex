#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Core)
set (MODULE_TYPE SHARED)
set (SOURCES 
    LayerStack.cpp
    Log/Log.cpp
)

if(UNIX)
  set(SOURCES
      ${SOURCES}
      Memory/Posix/PosixMemory.cpp
      Posix/PosixProcess.cpp
      Posix/PosixSharedLibrary.cpp
      Posix/PosixTime.cpp)
elseif (WIN32)
  set(SOURCES
      ${SOURCES} 
      Memory/Win32/Win32Memory.cpp
      Memory/Win32/Win32SharedLibrary.cpp
      Win32/Win32Time.cpp)
endif ()

# Prism is a shared library on the host (cmake/Dependencies.cmake), so every
# library and executable linking Vortex-Core shares one copy of it. Never
# whole-archive a static Prism into more than one image: each would get its
# own Prism globals and type info.
set (LIBS prism)

if (ANDROID)
    set (LIBS Prism log)
endif ()

vt_add_module (${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE SHARED
    DEFINITIONS_PUBLIC VT_CORE_SHARED_LIBRARY=1
    DEFINITIONS_PRIVATE VT_CORE_BUILD_DLL=1
    PUBLIC_LIBS Vortex-Modules ${LIBS} glm
)

# C++20 module interface units of Vortex-Core.
target_sources (${MODULE_NAME} PUBLIC FILE_SET CXX_MODULES FILES
    Formatters.ixx
)
