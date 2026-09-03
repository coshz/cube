find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(Python3_EXECUTABLE ${Python3_EXECUTABLE} PARENT_SCOPE)

set(pycube_staging_dir "${CMAKE_CURRENT_BINARY_DIR}/staging")

file(WRITE "${pycube_staging_dir}/pyproject.toml" [=[
[build-system]
requires=["setuptools>=61.0"]
build-backend="setuptools.build_meta"
]=])

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in"
    "${pycube_staging_dir}/setup.py"
    @ONLY
)

add_custom_target(pycube ALL 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/pycube"
        "${pycube_staging_dir}/pycube"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "$<TARGET_FILE:${PROJECT_NAME}::core>" 
        "${pycube_staging_dir}/pycube/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "${pycube_staging_dir}/README.md"
    DEPENDS ${PROJECT_NAME}::core
    COMMENT "Assembling pycube package and metadata in staging directory"
    VERBATIM
)

if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    list(LENGTH CMAKE_OSX_ARCHITECTURES macos_arch_count)
    if(macos_arch_count GREATER 1)
        set(macos_ver_arch "macosx-${CMAKE_OSX_DEPLOYMENT_TARGET}-universal2")
    else()
        set(macos_ver_arch "macosx-${CMAKE_OSX_DEPLOYMENT_TARGET}-${CMAKE_OSX_ARCHITECTURES}")
    endif()
    set(macos_whl_env "_PYTHON_HOST_PLATFORM=${macos_ver_arch}")
else()
    set(macos_whl_env "")
endif()

add_custom_target(pycube_whl
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CUBE_PACK_DIR}"
    COMMAND ${CMAKE_COMMAND} -E env ${macos_whl_env}
            ${Python3_EXECUTABLE} -m build --wheel --outdir "${CUBE_PACK_DIR}"
    WORKING_DIRECTORY "${pycube_staging_dir}"
    DEPENDS pycube 
    COMMENT "Building pycube wheel package"
    VERBATIM
)

add_dependencies(${CUBE_PACK} pycube_whl)

# if(NOT DEFINED PYTHON_SITE_PACKAGES)
#     execute_process(
#         COMMAND ${Python3_EXECUTABLE} -c 
#             "import sysconfig,sys,os; print(os.path.relpath(sysconfig.get_path('platlib'),sys.prefix))"
#         OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
#         OUTPUT_STRIP_TRAILING_WHITESPACE
#     )
# endif()

# install(
#     DIRECTORY "${pycube_staging_dir}"
#     DESTINATION "${PYTHON_SITE_PACKAGES}"
#     FILES_MATCHING 
#         PATTERN "*.py"
#         PATTERN "*.dylib"
#         PATTERN "*.so"
#         PATTERN "*.dll"
# )