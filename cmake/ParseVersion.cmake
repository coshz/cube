function(parse_version_from_file VERSION_FILE)
    # version format: v{major}.{minor}.{patch}-{build}
    file(READ ${VERSION_FILE} RAW_VERSION)
    string(STRIP "${RAW_VERSION}" RAW_VERSION)

    set(CUBE_VERSION_FULL "${RAW_VERSION}" PARENT_SCOPE)
    
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

    if(NOT BUILD_META)
        set(BUILD_META "")
    endif()

    set(CUBE_VERSION_MAJOR "${MAJOR}" PARENT_SCOPE)
    set(CUBE_VERSION_MINOR "${MINOR}" PARENT_SCOPE)
    set(CUBE_VERSION_PATCH "${PATCH}" PARENT_SCOPE)
    set(CUBE_VERSION_BUILD "${BUILD_META}" PARENT_SCOPE)
    set(CUBE_VERSION_SEMVER "${SEMVER}" PARENT_SCOPE)
endfunction()