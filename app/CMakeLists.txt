set(cli_sources 
    cli.cpp
)

add_executable(icube ${cli_sources})

target_link_libraries(icube PRIVATE ${PROJECT_NAME}::static)

install(TARGETS icube)