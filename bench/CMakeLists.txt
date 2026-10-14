set(bench_sources
    bench.cpp
)
add_executable(bench ${bench_sources})
target_link_libraries(bench PRIVATE ${PROJECT_NAME}::shared)