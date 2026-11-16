cmake_minimum_required(VERSION 3.28)

if(APPLE)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment target")
    endif()
    if(BUILD_SWIFT_MODULE AND NOT CMAKE_GENERATOR STREQUAL "Ninja")
        message(FATAL_ERROR "ninja is required if BUILD_SWIFT_MODULE is enabled")
    endif()
endif()

# version info
include("cmake/Modules/CubeVersion.cmake")
configure_version(CUBE
    FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
)

project(cube
    VERSION ${CUBE_VERSION}
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "@ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type")
set(CUBE_PACK_DIR "${PROJECT_BINARY_DIR}/dist" CACHE STRING "Cube packaging directory")

option(BUILD_STATIC_LIBS "Build static library" ON)
option(BUILD_SWIFT_MODULE "Build Cube.framework" ON)
option(CUBE_ENABLE_TRACE "Enable tracing table generation" $<IF:$<CONFIG:Debug>,ON,OFF>)
option(ENABLE_TEST "Enable testing" OFF)

# ------------------------------------------------------------------------------
# Generated headers
# ------------------------------------------------------------------------------

set(CUBE_BUILD_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include" )

set(cube_public_headers
    cube.h
    cube.hh
    advanced.hh
    cube_export.h
)

foreach(header IN LISTS cube_public_headers)
    configure_file(
        "${PROJECT_SOURCE_DIR}/include/cube/${header}"
        "${CUBE_BUILD_INCLUDE_DIR}/cube/${header}"
        COPYONLY
    )
endforeach()

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/Templates/version.h.in"
    "${CUBE_BUILD_INCLUDE_DIR}/cube/version.h"
    @ONLY
)

# ------------------------------------------------------------------------------
# Project Pack
# ------------------------------------------------------------------------------
set(system_proc "${CMAKE_SYSTEM_PROCESSOR}")
if(APPLE)
    if(CMAKE_OSX_ARCHITECTURES)
        list(LENGTH CMAKE_OSX_ARCHITECTURES arch_count)
        if(arch_count GREATER 1)
            set(system_proc "universal")
        else()
            set(system_proc "${CMAKE_OSX_ARCHITECTURES}" )
        endif()
    endif()
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-sdk")
set(CPACK_PACKAGE_DIRECTORY ${CUBE_PACK_DIR})
string(TOLOWER "${CPACK_PACKAGE_NAME}-${CUBE_VERSION}-${CMAKE_SYSTEM_NAME}-${system_proc}" CPACK_PACKAGE_FILE_NAME)

if(NOT CPACK_GENERATOR)
    if(WIN32)
        set(CPACK_GENERATOR "ZIP")
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
endif()

include(CPack)

# ------------------------------------------------------------------------------
# Build
# ------------------------------------------------------------------------------

add_subdirectory(src)   # library
add_subdirectory(app)   # executable

if(BUILD_SWIFT_MODULE AND APPLE)
    add_subdirectory(bindings/swift)
endif()

if(ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

# ------------------------------------------------------------------------------
# Package
# ------------------------------------------------------------------------------

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/CubeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/Templates/CubeConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/CubeConfig.cmake"
    INSTALL_DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/cube"
)

install(
    EXPORT "${PROJECT_NAME}Targets"
    FILE CubeTargets.cmake
    NAMESPACE Cube::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cube"
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/CubeConfig.cmake"
        "${PROJECT_BINARY_DIR}/CubeConfigVersion.cmake"
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/cube"
)

# ------------------------------------------------------------------------------
# pkg-config
# ------------------------------------------------------------------------------

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/Templates/cube.pc.in"
    "${PROJECT_BINARY_DIR}/cube.pc"
    @ONLY
)

install(
    FILES "${PROJECT_BINARY_DIR}/cube.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

# ------------------------------------------------------------------------------
# Configuration Summary
# ------------------------------------------------------------------------------

message(STATUS "--------------------------------------------------------------")
message(STATUS "The Configuration Summary of ${PROJECT_NAME}")
message(STATUS "  Project Verison: ${PROJECT_VERSION}")
message(STATUS "  Build Version  : ${CUBE_VERSION_FULL}")
message(STATUS "--------------------------------------------------------------")
message(STATUS "Environment:")
message(STATUS "  OS                    : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Camke Generator       : ${CMAKE_GENERATOR}")
message(STATUS "  C++ Standard          : ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  Bulid Static Libraries: ${BUILD_STATIC_LIBS}")
message(STATUS "  Build Swift Module    : ${BUILD_SWIFT_MODULE}")
message(STATUS "  Trace Output          : ${CUBE_ENABLE_TRACE}")
message(STATUS "  Enable Testing        : ${ENABLE_TEST}")
message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  Build Type            : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Directory       : ${PROJECT_BINARY_DIR}")
message(STATUS "  Install Prefix        : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Package Directory     : ${CUBE_PACK_DIR}")
message(STATUS "")
message(STATUS "Available Commands:")
message(STATUS "  Configure (See 'Options' and 'Build Configuration' above)")
message(STATUS "    'cmake -B <build> -D<key>=<value>'")
message(STATUS "  Build (All targets: {all(default),cube_shared,cube_static,icube})")
message(STATUS "    'cmake --build <build>'")
message(STATUS "    'cmake --build <build> --target <target>'")
message(STATUS "  Install")
message(STATUS "    'cmake --install <build>'")
message(STATUS "    'cmake --install <build> --prefix <prefix>'")
message(STATUS "  Package")
message(STATUS "    'cmake --build <build> --target package'")
message(STATUS "--------------------------------------------------------------")
