include(FetchContent)
FetchContent_Declare(
    googletest 
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(../src)

add_executable(cube_test cube_test.cpp ../src/cube.cpp)
target_link_libraries(cube_test GTest::gtest_main)

add_executable(libcube_test libcube_test.cpp)
target_link_libraries(libcube_test cube GTest::gtest_main)

include(GoogleTest)

# [bug & workaround: https://github.com/google/googletest/issues/3475 ]
# gtest_discover_tests(libcube_test)

gtest_add_tests(TARGET cube_test)
gtest_add_tests(TARGET libcube_test)