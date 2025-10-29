include(FetchContent)
FetchContent_Declare(
    googletest 
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(../src)

add_executable(maneuver_test maneuver_test.cpp ../src/maneuver.cpp)
target_link_libraries(maneuver_test GTest::gtest_main)

add_executable(cube_test cube_test.cpp)
target_link_libraries(cube_test GTest::gtest_main libcube)

add_executable(coord_test coord_test.cpp)
target_link_libraries(coord_test GTest::gtest_main libcube)

include(GoogleTest)

# [bug & workaround: https://github.com/google/googletest/issues/3475 ]
# gtest_discover_tests(maneuver_test)
# gtest_discover_tests(cube_test)
# gtest_discover_tests(coord_test)
gtest_add_tests(TARGET maneuver_test)
gtest_add_tests(TARGET cube_test)
gtest_add_tests(TARGET coord_test)
