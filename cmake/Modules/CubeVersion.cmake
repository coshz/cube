include_guard(GLOBAL)

function(configure_cube_version)
    set(one_value_args FILE STRING)
    cmake_parse_arguments(ARG 
        "" 
        "${one_value_args}" "" 
        ${ARGN}
    )
    if(ARG_STRING)
        set(RAW_VERSION ${ARG_STRING})
    elseif(ARG_FILE)
        if(NOT EXISTS ${ARG_FILE})
            message(FATAL_ERROR "Version file not found: ${ARG_FILE}")
        endif()
        file(READ "${ARG_FILE}" RAW_VERSION)
        string(STRIP "${RAW_VERSION}" RAW_VERSION)
    else()
        message(FATAL_ERROR "invalid argument: one of FILE or STRING must be provided")
    endif()
    
    # parse version
    string(REGEX REPLACE "^v" "" CLEAN_VERSION "${RAW_VERSION}")
    string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" SEMVER "${CLEAN_VERSION}")

    if(NOT SEMVER)
        message(FATAL_ERROR "Invalid version format in ${VERSION_FILE}: ${RAW_VERSION}")
    endif()

    string(REPLACE "." ";" VERSION_LIST "${SEMVER}")
    list(GET VERSION_LIST 0 MAJOR)
    list(GET VERSION_LIST 1 MINOR)
    list(GET VERSION_LIST 2 PATCH)

    string(REGEX MATCH "-([A-Za-z0-9]+)$" _ "${CLEAN_VERSION}")
    set(BUILD_META "${CMAKE_MATCH_1}")

    set(CUBE_VERSION_FULL   ${RAW_VERSION}  PARENT_SCOPE)
    set(CUBE_VERSION_SEMVER ${SEMVER}       PARENT_SCOPE)
    set(CUBE_VERSION_MAJOR  ${MAJOR}        PARENT_SCOPE)
    set(CUBE_VERSION_MINOR  ${MINOR}        PARENT_SCOPE)
    set(CUBE_VERSION_PATCH  ${PATCH}        PARENT_SCOPE)
    set(CUBE_VERSION_BUILD  ${BUILD_META}   PARENT_SCOPE)
endfunction()