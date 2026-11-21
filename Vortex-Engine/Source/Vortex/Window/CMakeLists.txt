#
# Created by v1tr10l7 on 18.06.2026. Copyright (c) 2024-2026, Szymon Zemke
# <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-3
#
set (MODULE_NAME Vortex-Window)
set (MODULE_TYPE SHARED)
set (SOURCES 
    Monitor.cpp
    Window.cpp
    Input/Keyboard.cpp
    Input/Mouse.cpp
)

set (LIBS Vortex-Modules Vortex-Core)

if(UNIX)
  if (NOT ANDROID)
    set(SOURCES
      ${SOURCES}
      Wayland/Wayland.cpp
      Wayland/WaylandMonitor.cpp
      Wayland/WaylandWindow.cpp
      X11/X11.cpp
      X11/X11Monitor.cpp
      X11/X11Window.cpp
      X11/XkbUnicode.cpp
    )
    set(LIBS ${LIBS} xcb X11 X11-xcb xkbcommon wayland-client wayland-cursor)
  else ()
    set (LIBS ${LIBS} android log nativewindow)
  endif ()
elseif (WIN32)
  set(SOURCES
      ${SOURCES} 
      Win32/Win32Monitor.cpp
      Win32/Win32Window.cpp
  )

  set (LIBS ${LIBS} Shcore)
endif ()

if (NOT ANDROID)
    set (LIBS ${LIBS} nfd)
endif ()
 
vt_add_module (${MODULE_NAME}
    SOURCES ${SOURCES}
    TYPE SHARED
    DEFINITIONS_PUBLIC SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=1
    DEFINITIONS_PRIVATE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    PUBLIC_LIBS ${LIBS}
)
if(VT_BUILD_WAYLAND AND NOT ANDROID)
    set(WAYLAND_GENERATED_FILES "")

    macro(generate_wayland_protocol protocol_file)
      set(protocol_path "${VENDOR_DIR}/wayland-protocols/${protocol_file}")

      string(REGEX REPLACE "\\.xml$" "-client-protocol.h" header_file ${protocol_file})
      string(REGEX REPLACE "\\.xml$" "-client-protocol-code.c" code_file ${protocol_file})
      
      set(header_file "${VORTEX_WAYLAND_GEN_DIR}/wayland-${header_file}")
      set(code_file "${VORTEX_WAYLAND_GEN_DIR}/wayland-${code_file}")

      add_custom_command(
        OUTPUT "${header_file}"
        COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_path}" "${header_file}"
        DEPENDS "${protocol_path}"
        VERBATIM)

      add_custom_command(
        OUTPUT "${code_file}"
        COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_path}" "${code_file}"
        DEPENDS "${protocol_path}"
        VERBATIM)


      list(APPEND WAYLAND_GENERATED_FILES "${header_file}" "${code_file}")  
      target_sources(${MODULE_NAME} PRIVATE ${header_file} ${code_file})
    endmacro()

    generate_wayland_protocol("wayland.xml")
    generate_wayland_protocol("xdg-shell.xml")
    generate_wayland_protocol("alpha-modifier-v1.xml")
    generate_wayland_protocol("wlr-gamma-control-unstable-v1.xml")
    generate_wayland_protocol("xdg-toplevel-icon-v1.xml")
    # Pointer lock and raw mouse motion (first-person mouse look).
    generate_wayland_protocol("relative-pointer-unstable-v1.xml")
    generate_wayland_protocol("pointer-constraints-unstable-v1.xml")

    message(STATUS "Generated Wayland protocol files: ${WAYLAND_GENERATED_FILES}")
endif()

if(UNIX AND NOT ANDROID AND WAYLAND_GENERATED_FILES)
  set_source_files_properties(${WAYLAND_GENERATED_FILES} PROPERTIES
    COMPILE_FLAGS "-fvisibility=default"
  )
target_include_directories(Vortex-Window PRIVATE "${VORTEX_WAYLAND_GEN_DIR}")
endif()
