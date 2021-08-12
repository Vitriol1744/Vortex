# Vortex-Engine

cmake_minimum_required (VERSION 3.10)
project (Vortex-Engine)

find_package (OpenGL REQUIRED COMPONENTS OpenGL)

#TODO: Download FMT

set (CMAKE_CXX_STANDARD 20)
set (CXX_STANDARD_REQUIRED ON)

add_subdirectory (vendor/fmt)
add_subdirectory (vendor/OpenAL)
add_subdirectory (vendor/glm)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions (-DDEBUG)
endif()

add_definitions (-DVT_BUILD_DLL)
add_definitions (-DVT_ENABLE_ASSERTIONS)

set (WIN32_EXECUTABLE TRUE)
set (VENDOR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set (INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/Vortex" "${VENDOR}/OpenGL/include" "${VENDOR}/OpenAL/include" "fmt" "${VENDOR}/glm")

set (LIBS "OpenGL::GL" "fmt" "OpenAL" "glm")
file (GLOB_RECURSE SOURCES "src/*/*.cpp")

if (UNIX)
    set (LIBS ${LIBS} X11)
endif()

add_library (${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries (${PROJECT_NAME} PUBLIC ${LIBS})
#target_precompile_headers (${PROJECT_NAME} PRIVATE "src/Vortex/vtpch.hpp")

#if (_WIN32)
#add_custom_command (
#    TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_BINARY_DIR}/Vortex-Engine/Vortex-Engine.dll ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})
#elseif (UNIX)
#add_custom_command (
#   TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
#   ${CMAKE_BINARY_DIR}/Vortex-Engine/libVortex-Engine.so ${CMAKE_BINARY_DIR}/XSandbox/ DEPENDS Vortex-Engine})
#endif ()

#add_custom_command (
#    TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}/assets)
