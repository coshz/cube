set(CPP_SOURCES 
    core.cpp 
    twophase.cpp 
    table.cpp 
    coord.cpp 
    cube.cpp
)

list(TRANSFORM CPP_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(CPP_SOURCES ${CPP_SOURCES} PARENT_SCOPE)

include(GenerateExportHeader)

set(cube_export_macro "cube_EXPORTS")

# ------------------------------------------------------------------------------
# Internal object library
# The C++ sources are compiled exactly once and their objects are reused by:
#   - ${PROJECT_NAME}::core
#   - ${PROJECT_NAME}::Framework
# ------------------------------------------------------------------------------

add_library(cube_objs OBJECT ${CPP_SOURCES})
add_library(${PROJECT_NAME}::objs ALIAS cube_objs)

target_include_directories(cube_objs PUBLIC
    $<BUILD_INTERFACE:${CUBE_BUILD_INCLUDE_DIR}>
)

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
# Main C++ library
#   - shared: primary
#   - static: optional
# ------------------------------------------------------------------------------

# Shared library

add_library(cube_shared SHARED)

add_library(${PROJECT_NAME}::core ALIAS cube_shared)
add_library(${PROJECT_NAME}::shared ALIAS cube_shared)

target_link_libraries(cube_shared PRIVATE ${PROJECT_NAME}::objs)

set_target_properties(cube_shared PROPERTIES
    OUTPUT_NAME     cube
    VERSION         ${PROJECT_VERSION}
    SOVERSION       ${PROJECT_VERSION_MAJOR}
    DEFINE_SYMBOL   "${cube_export_macro}"
)

# Generate export header
generate_export_header(cube_shared
    BASE_NAME CUBE
    EXPORT_MACRO_NAME CUBE_EXPORT
    EXPORT_FILE_NAME "${CUBE_BUILD_INCLUDE_DIR}/cube/cube_export.h"
)

target_sources(cube_shared
PUBLIC
    FILE_SET HEADERS 
    BASE_DIRS 
        ${CUBE_BUILD_INCLUDE_DIR}
    FILES 
        "${CUBE_BUILD_INCLUDE_DIR}/cube/cube.h"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/cube_export.h"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/version.h"
)

set(cube_libs cube_shared)

# Static library

if(BUILD_STATIC_LIBS)
    add_library(cube_static STATIC ${CPP_SOURCES})
    add_library(${PROJECT_NAME}::static ALIAS cube_static)
    target_include_directories(cube_static PUBLIC
        $<BUILD_INTERFACE:${CUBE_BUILD_INCLUDE_DIR}>
    )
    
    if(WIN32)
        set_target_properties(cube_static PROPERTIES
            OUTPUT_NAME "cube_static"
        )
    else()
        set_target_properties(cube_static PROPERTIES
            OUTPUT_NAME "cube"
        )
    endif()
    
    target_compile_definitions(cube_static PUBLIC CUBE_STATIC_DEFINE)
    target_sources(cube_static PUBLIC
        FILE_SET HEADERS 
        BASE_DIRS 
            ${CUBE_BUILD_INCLUDE_DIR}
        FILES 
            "${CUBE_BUILD_INCLUDE_DIR}/cube/cube.h"
            "${CUBE_BUILD_INCLUDE_DIR}/cube/cube_export.h"
            "${CUBE_BUILD_INCLUDE_DIR}/cube/version.h"
    )
    list(APPEND cube_libs cube_static)
endif()

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------

install(
    TARGETS ${cube_libs} 
    EXPORT "${PROJECT_NAME}Targets"
    FILE_SET HEADERS
)