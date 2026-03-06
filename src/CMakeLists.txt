set(cube_sources 
    twophase.cpp table.cpp coord.cpp cube.cpp
)

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/cube_export.h.in
    ${CUBE_GENERATED_HEADERS_DIR}/cube_export.h
)

add_library(cube SHARED core.cpp ${cube_sources})

set_target_properties(cube PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(cube PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CUBE_GENERATED_HEADERS_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(cube PRIVATE 
    CUBE_BUILD
    VERBOSE=0
)

target_compile_options(cube PRIVATE 
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-fvisibility=hidden>
)

install(TARGETS cube
    EXPORT cube_targets 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES ${CUBE_GENERATED_HEADERS_DIR}/cube_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)