set(cli_sources 
    cli.cpp
)
add_executable (icube ${cli_sources})
target_link_libraries(icube cube)

if(VERBOSE)
    target_compile_definitions(cube PRIVATE VERBOSE=1)
endif()

if(APPLE)
    set_target_properties(icube PROPERTIES
        INSTALL_RPATH "@loader_path/../lib"
    )
elseif(UNIX)
    set_target_properties(icube PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

install(TARGETS icube DESTINATION ${CMAKE_INSTALL_BINDIR})