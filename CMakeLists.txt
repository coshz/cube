cmake_minimum_required(VERSION 3.28)

if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Supported macOS architectures")
    endif()
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment target")
    endif()

    if(BUILD_SWIFT_MODULE AND NOT CMAKE_GENERATOR STREQUAL "Ninja")
        message(FATAL_ERROR "ninja is required if BUILD_SWIFT_MODULE is enabled")
    endif()
endif()

# version info (read from file / string)
include("cmake/Modules/CubeVersion.cmake")
configure_cube_version(
    FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
)

project(cube 
    VERSION ${CUBE_VERSION_SEMVER} 
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

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

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(CUBE_PACK_DIR "${PROJECT_BINARY_DIR}/dist" CACHE STRING "Cube packaging directory")

option(BUILD_STATIC_LIBS "Build cube static library" ON)
option(BUILD_SWIFT_MODULE "Build Apple Cube.framework and Swift module map" OFF)
option(BUILD_PYTHON_BINDING "Build Python binding" OFF)
option(BUILD_WASM_BINDING "Build WASM binding" OFF)
option(ENABLE_TEST "Enable testing" OFF)

# ------------------------------------------------------------------------------
# Generated headers
# ------------------------------------------------------------------------------

set(CUBE_BUILD_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include" )

configure_file(
    "${PROJECT_SOURCE_DIR}/include/cube/cube.h"
    "${CUBE_BUILD_INCLUDE_DIR}/cube/cube.h"
    COPYONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/Templates/version.h.in"
    "${CUBE_BUILD_INCLUDE_DIR}/cube/version.h"
    @ONLY
)

# ------------------------------------------------------------------------------
# Project Pack
# ------------------------------------------------------------------------------
string(TOLOWER ${CMAKE_SYSTEM_NAME} system_name)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} system_proc)

if(APPLE)
    if(CMAKE_OSX_ARCHITECTURES)
        list(LENGTH CMAKE_OSX_ARCHITECTURES arch_count)
        if(arch_count GREATER 1)
            set(system_proc "universal")
        else()
            string(TOLOWER ${CMAKE_OSX_ARCHITECTURES} system_proc)
        endif()
    endif()
endif()

set(CUBE_PACK "pack")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-sdk")
set(CPACK_PACKAGE_DIRECTORY ${CUBE_PACK_DIR})
set(CPACK_PACKAGE_FILE_NAME 
    "${CPACK_PACKAGE_NAME}-${CUBE_VERSION_SEMVER}-${system_name}-${system_proc}"
)

if(NOT CPACK_GENERATOR)
    if(WIN32)
        set(CPACK_GENERATOR "ZIP")
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
endif()

set(CPACK_REMOVE_TOPLEVEL_DIRECTORY ON)

include(CPack)

add_custom_target(${CUBE_PACK} 
    COMMAND ${CMAKE_CPACK_COMMAND} -C $<CONFIG>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running CPack to package primary installation artifacts"
    VERBATIM
)

# ------------------------------------------------------------------------------
# Build 
# ------------------------------------------------------------------------------

add_subdirectory(src)   # library
add_subdirectory(app)   # executable

if(BUILD_SWIFT_MODULE AND APPLE)
    add_subdirectory(bindings/swift)
endif()

if(BUILD_PYTHON_BINDING)
    add_subdirectory(bindings/python)
endif()

if(BUILD_WASM_BINDING)
    add_subdirectory(bindings/wasm)
endif()

if(ENABLE_TEST)
    enable_testing() 
    add_subdirectory(test)
endif()

# ------------------------------------------------------------------------------
# Cmake package
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

# ----------------------------------------------------------------------------
# Configuration Summary
# ------------------------------------------------------------------------------

message(STATUS "--------------------------------------------------------------")
message(STATUS "  ${PROJECT_NAME} v${PROJECT_VERSION} Configuration Summary")
message(STATUS "--------------------------------------------------------------")
message(STATUS "")
message(STATUS "Environment:")
message(STATUS "  OS                    : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Camke Generator       : ${CMAKE_GENERATOR}")
message(STATUS "  C++ Compiler          : ${CMAKE_CXX_COMPILER}")
if(BUILD_PYTHON_BINDING)
message(STATUS "  Python Interpreter    : ${Python3_EXECUTABLE}")
endif()
if(BUILD_WASM_BINDING)
message(STATUS "  Emscripten            : ${EMCC_EXECUTABLE}")
endif()
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  Bulid Static Libraries: ${BUILD_STATIC_LIBS}")
message(STATUS "  Build Swift Module    : ${BUILD_SWIFT_MODULE}")
message(STATUS "  Build Python Binding  : ${BUILD_PYTHON_BINDING}")
message(STATUS "  Build WASM Binding    : ${BUILD_WASM_BINDING}")
message(STATUS "  Enable Testing        : ${ENABLE_TEST}")
message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  C++ Standard          : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type            : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Directory       : ${PROJECT_BINARY_DIR}")
message(STATUS "  Install Prefix        : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Package Directory     : ${CUBE_PACK_DIR}")
message(STATUS "")
message(STATUS "Available Targets:")
message(STATUS "  all (default)         : Build the project")
message(STATUS "  install               : Install built artifacts to CMAKE_INSTALL_PREFIX")
message(STATUS "  pack                  : Package sdk, pycube and jscube")
message(STATUS "")
message(STATUS "Use")
message(STATUS "  'cmake -B <build> -D<key>=<value>' to configure project;")
message(STATUS "  'cmake --build <build> --target <target>' to execute build tasks.")
message(STATUS "--------------------------------------------------------------")