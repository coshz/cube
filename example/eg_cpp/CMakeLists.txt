cmake_minimum_required(VERSION 3.28)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

find_package(Cube REQUIRED)

add_executable(my_app eg.cpp)
target_link_libraries(my_app PRIVATE Cube::shared)