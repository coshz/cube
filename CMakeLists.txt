cmake_minimum_required(VERSION 3.24)
project(Cube VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)   # library
add_subdirectory(app)   # executable 

option(VERBOSE "print table information" OFF)
option(ENABLE_TEST "enable testing" OFF)

if(ENABLE_TEST)
    enable_testing() 
    add_subdirectory(test)
endif()

include(GNUInstallDirs)

# headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# export targets
install(EXPORT cube_targets 
    FILE CubeTargets.cmake 
    NAMESPACE Cube:: 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cube
)

# generate CubeConfig.cmake
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/CubeConfig.cmake.in 
    ${CMAKE_CURRENT_BINARY_DIR}/CubeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cube
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

# python: just copy
install(
    DIRECTORY python
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}
)

# wasm: just copy
install(
    DIRECTORY wasm
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}
)

install(TARGETS cube
    LIBRARY DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/python/cube_ctypes
)