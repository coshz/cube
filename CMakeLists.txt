cmake_minimum_required(VERSION 3.24)

include(cmake/ParseVersion.cmake)
parse_version_from_file(VERSION)

project(cube VERSION ${CUBE_VERSION_SEMVER} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CUBE_GENERATED_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/inc_gen")

include(GNUInstallDirs)

add_subdirectory(src)   # library
add_subdirectory(app)   # executable 

option(VERBOSE "print table information" OFF)
option(ENABLE_TEST "enable testing" OFF)

if(ENABLE_TEST)
    enable_testing() 
    add_subdirectory(test)
endif()

# generate version info
configure_file(
    "cmake/version.h.in"
    ${CUBE_GENERATED_HEADERS_DIR}/version.h
    @ONLY
)

# install headers & library
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
    FILES ${CUBE_GENERATED_HEADERS_DIR}/version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
install(EXPORT cube_targets 
    FILE CubeTargets.cmake 
    NAMESPACE Cube:: 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cube
)

# generate CubeConfig.cmake
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/CubeConfig.cmake.in 
    ${CMAKE_CURRENT_BINARY_DIR}/CubeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cube
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CubeConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# generate cube.pc 
configure_file(
    cmake/cube.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/cube.pc
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cube.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)