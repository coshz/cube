set(CPP_SOURCES
    cube_impl.cpp
    cube_impl_c.cpp
    config.cpp
    maneuver.cpp
    rubik.cpp
    twophase.cpp
    table.cpp
    coord.cpp
)

add_library(cube_headers INTERFACE)
add_library(cube_objs OBJECT ${CPP_SOURCES})
add_library(cube_shared SHARED)

add_library(${PROJECT_NAME}::objs ALIAS cube_objs)
add_library(${PROJECT_NAME}::core ALIAS cube_shared)
add_library(${PROJECT_NAME}::shared ALIAS cube_shared)

set(cube_install_targets cube_headers cube_shared)

# Generate exported header

set(cube_export_macro "cube_EXPORTS")
include(GenerateExportHeader)
generate_export_header(cube_shared
    BASE_NAME CUBE
    EXPORT_MACRO_NAME CUBE_EXPORT
    EXPORT_FILE_NAME "${CUBE_BUILD_INCLUDE_DIR}/cube/cube_export.h"
)

# ------------------------------------------------------------------------------
# Public header
# ------------------------------------------------------------------------------

target_sources(cube_headers INTERFACE 
    FILE_SET HEADERS 
    BASE_DIRS
        ${CUBE_BUILD_INCLUDE_DIR}
    FILES
        "${CUBE_BUILD_INCLUDE_DIR}/cube/cube_export.h"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/version.h"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/cube.h"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/cube.hh"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/advanced.hh"
)

# ------------------------------------------------------------------------------
# Internal object library
# The C++ sources are compiled exactly once and their objects are reused by:
#   - ${PROJECT_NAME}::core
#   - ${PROJECT_NAME}::Framework
# ------------------------------------------------------------------------------

target_link_libraries(cube_objs PUBLIC cube_headers)

# ------------------------------------------------------------------------------
# [Windows / MSVC Workaround]
# OBJECT libraries do not automatically inherit DEFINE_SYMBOL from downstream
# SHARED targets.
#
# Explicitly define the export macro so cube_export.h expands CUBE_EXPORT
# to __declspec(dllexport) instead of __declspec(dllimport) when compiling .cpp files.
#
# WARNING: Omitting this line causes MSVC warning C4273 ("inconsistent dll linkage")
# and generates an empty import library (cube.lib), leading to LNK2019 unresolved
# external symbol errors in dependent targets (e.g., icube.exe).
# ------------------------------------------------------------------------------
target_compile_definitions(cube_objs PRIVATE ${cube_export_macro})

# ------------------------------------------------------------------------------
# Main C++ library (shared & static)
# ------------------------------------------------------------------------------

target_link_libraries(cube_shared 
    PRIVATE ${PROJECT_NAME}::objs
    PUBLIC cube_headers
)

set_target_properties(cube_shared PROPERTIES
    OUTPUT_NAME     cube
    VERSION         ${PROJECT_VERSION}
    SOVERSION       ${PROJECT_VERSION_MAJOR}
    DEFINE_SYMBOL   "${cube_export_macro}"
)

if(BUILD_STATIC_LIBS)
    # build static library from source !!!
    add_library(cube_static STATIC ${CPP_SOURCES})
    add_library(${PROJECT_NAME}::static ALIAS cube_static)
   
    if(WIN32)
        set_target_properties(cube_static PROPERTIES OUTPUT_NAME "cube_static")
    else()
        set_target_properties(cube_static PROPERTIES OUTPUT_NAME "cube")
    endif()
    target_compile_definitions(cube_static PUBLIC CUBE_STATIC_DEFINE)
    target_link_libraries(cube_static PUBLIC cube_headers)

    list(APPEND cube_install_targets cube_static)
endif()

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------

install(
    TARGETS ${cube_install_targets}
    EXPORT "${PROJECT_NAME}Targets"
    FILE_SET HEADERS
)