find_program(EMCC_EXECUTABLE em++ REQUIRED)
find_program(NPM_EXECUTABLE npm REQUIRED)

set(EMCC_EXECUTABLE "${EMCC_EXECUTABLE}" PARENT_SCOPE)

set(jscube_export_name "createModule")
set(jscube_staging_dir "${CMAKE_CURRENT_BINARY_DIR}/staging")
set(jscube_internal_dir "${jscube_staging_dir}/dist/internal")

set(wasm_sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/native/cube_wasm.cpp"
    ${CPP_SOURCES}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(WASM_OPT_FLAGS "-O0")
else()
    set(WASM_OPT_FLAGS "-O2" "-DNDEBUG")
endif()

set(EM_COMPILE_FLAGS
    -r
    -std=c++17
    -I${CUBE_BUILD_INCLUDE_DIR}
    ${WASM_OPT_FLAGS}
)

set(EM_LINK_FLAGS
    -lembind
    -s EXPORT_NAME=${jscube_export_name}
    ${WASM_OPT_FLAGS}
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/package.json.in"
    "${jscube_staging_dir}/package.json"
    @ONLY
)

add_custom_target(jscube_staging ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${jscube_internal_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/tsconfig.json"
        "${jscube_staging_dir}/tsconfig.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${jscube_staging_dir}/README.md"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${jscube_staging_dir}/src"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/test"
        "${jscube_staging_dir}/test"
    COMMENT "Assembling jscube staging directory"
    VERBATIM
)

add_custom_command(
    OUTPUT "${jscube_staging_dir}/cube_wasm.o"
    COMMAND ${EMCC_EXECUTABLE} ${wasm_sources} ${EM_COMPILE_FLAGS}
        -o "${jscube_staging_dir}/cube_wasm.o"
    DEPENDS ${wasm_sources}
    COMMENT "Compiling C++ sources to Emscripten object file"
    VERBATIM
)

# jscube-web
add_custom_command(
    OUTPUT "${jscube_internal_dir}/jscube.web.mjs"
    COMMAND ${EMCC_EXECUTABLE} "${jscube_staging_dir}/cube_wasm.o" ${EM_LINK_FLAGS}
        -s ENVIRONMENT=web
        -s EXPORT_ES6=1
        -o "${jscube_internal_dir}/jscube.web.mjs"
    DEPENDS "${jscube_staging_dir}/cube_wasm.o"
    COMMENT "Linking WASM (Web)"
    VERBATIM
)

#jscube-node
add_custom_command(
    OUTPUT "${jscube_internal_dir}/jscube.node.cjs"
    COMMAND ${EMCC_EXECUTABLE} "${jscube_staging_dir}/cube_wasm.o" ${EM_LINK_FLAGS} 
        -s ENVIRONMENT=node
        -s MODULARIZE=1
        -s WASM_ASYNC_COMPILATION=0
        -o "${jscube_internal_dir}/jscube.node.cjs"
    DEPENDS "${jscube_staging_dir}/cube_wasm.o"
    COMMENT "Linking WASM (Node)"
    VERBATIM
)

file(GENERATE 
    OUTPUT "${jscube_internal_dir}/jscube.node.mjs"
    CONTENT [=[
import { createRequire } from "module";
const require = createRequire(import.meta.url);
export default require("./jscube.node.cjs");
]=]
)

add_custom_target(jscube_wasm
    DEPENDS 
        "${jscube_internal_dir}/jscube.web.mjs"
        "${jscube_internal_dir}/jscube.node.cjs"
        "${jscube_internal_dir}/jscube.node.mjs"
)

add_custom_target(jscube_pkg 
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CUBE_PACK_DIR}"
    COMMAND ${NPM_EXECUTABLE} install
    COMMAND ${NPM_EXECUTABLE} pack --pack-destination "${CUBE_PACK_DIR}"
    WORKING_DIRECTORY "${jscube_staging_dir}"
    DEPENDS jscube_wasm
    COMMENT "Build Typescript and packing npm package into ${CUBE_PACK_DIR}"
    VERBATIM
)

add_dependencies(${CUBE_PACK} jscube_pkg)