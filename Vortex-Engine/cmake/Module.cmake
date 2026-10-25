#*
#* Created by v1tr10l7 on 11.07.2026.
#* Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#*
#* SPDX-License-Identifier: GPL-3
#*
include(GenerateExportHeader)

function(vt_add_module TARGET_NAME)
    cmake_parse_arguments(ARG "" "TYPE;API_PREFIX" "SOURCES;DEFINITIONS_PUBLIC;DEFINITIONS_PRIVATE;PUBLIC_LIBS;PRIVATE_LIBS;PUBLIC_INCLUDES;PRIVATE_INCLUDES" ${ARGN})

    # Determine Target Type (SHARED, STATIC, or fallback to global BUILD_SHARED_LIBS)
    if(ARG_TYPE)
        set(TARGET_TYPE ${ARG_TYPE})
    elseif(BUILD_SHARED_LIBS)
        set(TARGET_TYPE SHARED)
    else()
        set(TARGET_TYPE STATIC)
    endif()    

    # Append shared library definitions automatically when building a SHARED library
    if(TARGET_TYPE STREQUAL "SHARED")
        list(APPEND ARG_DEFINITIONS_PUBLIC "VT_PROJECT_SHARED_LIBRARY=1")
        list(APPEND ARG_DEFINITIONS_PRIVATE "VT_PROJECT_BUILD_DLL=1")
    endif()

    # Create library target
    add_library(${TARGET_NAME} ${TARGET_TYPE} ${ARG_SOURCES})

    # Hide symbols and enable pic
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )

    # Calculate path relative to VORTEX_ENGINE_SOURCE_ROOT
    file(RELATIVE_PATH REL_MODULE_PATH "${VORTEX_ENGINE_SOURCE_ROOT}" "${CMAKE_CURRENT_SOURCE_DIR}")

    # Construct API Macro Name automatically if not explicitly provided
    if(NOT ARG_API_PREFIX)
        string(TOUPPER "${REL_MODULE_PATH}" UPPER_PATH)
        string(REGEX REPLACE "^VORTEX/" "" CLEAN_PATH "${UPPER_PATH}") # Remove leading VORTEX/ if present
        string(REGEX REPLACE "[/\\-]" "_" SANITIZED_PATH "${CLEAN_PATH}")
        set(API_MACRO_NAME "VT_${SANITIZED_PATH}_API")
    else()
        set(API_MACRO_NAME "${ARG_API_PREFIX}_API")
    endif()

    # Generate Export Header based on calculated path
    # Output: ${CMAKE_CURRENT_BINARY_DIR}/include/Vortex/Renderer/RHI/Vulkan/Export.hpp
    set(EXPORT_HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/${REL_MODULE_PATH}/Export.hpp")
    
    generate_export_header(${TARGET_NAME}
        EXPORT_FILE_NAME "${EXPORT_HEADER_PATH}"
        EXPORT_MACRO_NAME "${API_MACRO_NAME}"
    )

    # Expose the generated include directory
    target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")
    
    # Apply Compile Definitions & Include Directories
    if(ARG_DEFINITIONS_PUBLIC)
        target_compile_definitions(${TARGET_NAME} PUBLIC ${ARG_DEFINITIONS_PUBLIC})
    endif()
    if(ARG_DEFINITIONS_PRIVATE)
        target_compile_definitions(${TARGET_NAME} PRIVATE ${ARG_DEFINITIONS_PRIVATE})
    endif()
    if(ARG_PUBLIC_INCLUDES)
        target_include_directories(${TARGET_NAME} PUBLIC ${ARG_PUBLIC_INCLUDES})
    endif()
    if(ARG_PRIVATE_INCLUDES)
        target_include_directories(${TARGET_NAME} PRIVATE ${ARG_PRIVATE_INCLUDES})
    endif()

    # 7. Link Libraries
    if(ARG_PUBLIC_LIBS)
        target_link_libraries(${TARGET_NAME} PUBLIC ${ARG_PUBLIC_LIBS})
    endif()
    if(ARG_PRIVATE_LIBS)
        target_link_libraries(${TARGET_NAME} PRIVATE ${ARG_PRIVATE_LIBS})
    endif()
endfunction()
