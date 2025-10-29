cmake_minimum_required(VERSION 3.24)
project(TestCube CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src bin)

option(ENABLE_TEST "enable testing" OFF)
option(BUILD_SHARED_LIBS "build shared library" ON)

if(ENABLE_TEST)
    enable_testing() 
    add_subdirectory(test)
endif()

