set(cube_sources 
    twophase.cpp table.cpp coord.cpp cube.cpp
)

add_library(cube SHARED libcube.cpp ${cube_sources})
target_include_directories(cube PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(cube PRIVATE VERBOSE=0)

install(TARGETS cube
    EXPORT cube_targets 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)