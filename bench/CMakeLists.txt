cmake_minimum_required(VERSION 3.28)

project(bench 
    VERSION 0.1.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Cube QUIET)

if(Cube_FOUND)
    message(STATUS "Found Cube: ${Cube_VERSION} (at ${Cube_DIR})")
else()
    message(FATAL_ERROR "Failed to find Cube!\n"
                        "Please set Cube_DIR if it's not in a standard path.")
endif()

add_executable(bench src/bench.cpp)
target_link_libraries(bench PRIVATE Cube::shared)