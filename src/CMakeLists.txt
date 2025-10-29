set(cube_sources 
    maneuver.cpp twophase.cpp table.cpp coord.cpp cube.cpp
)

add_library(libcube SHARED libcube.cpp ${cube_sources})
target_compile_definitions(libcube PRIVATE VERBOSE=OFF)

add_executable (cube main.cpp)
target_link_libraries(cube libcube)
target_compile_definitions(cube PRIVATE VERBOSE=ON)