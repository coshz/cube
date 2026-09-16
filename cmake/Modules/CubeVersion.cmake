include_guard(GLOBAL)

#=============================================================================
# configure_version
#=============================================================================
#
# @brief Parses Semantic Versioning (SemVer) information and exports the
#        parsed version variables to the parent scope with a specified prefix.
#
# @param NAME_PREFIX [Required] Positional argument. Variable name prefix
#                    (e.g., CUBE, CUBE_PYTHON).
# @param FILE        [Optional] Keyword argument. Path to the version file.
# @param STRING      [Optional] Keyword argument. Version string provided directly.
#
# @note Exactly one of FILE or STRING must be provided. If both are passed,
#       STRING takes precedence.
#
# @details
# Assuming NAME_PREFIX is set to "CUBE", the following variables will be
# exported to the parent scope (PARENT_SCOPE):
#   - CUBE_VERSION_FULL   : Raw input version string (e.g., "v1.2.3-rc1")
#   - CUBE_VERSION_SEMVER : Standard X.Y.Z SemVer string (e.g., "1.2.3")
#   - CUBE_VERSION_MAJOR  : Major version number (e.g., "1")
#   - CUBE_VERSION_MINOR  : Minor version number (e.g., "2")
#   - CUBE_VERSION_PATCH  : Patch version number (e.g., "3")
#   - CUBE_VERSION_BUILD  : Build metadata / suffix (e.g., "rc1"; empty if none)
#
# @example
#   # Example 1: Read version from a file
#   configure_version(CUBE FILE "${CMAKE_CURRENT_SOURCE_DIR}/version")
#
#   # Example 2: Parse a version string directly
#   configure_version(CUBE_PYTHON STRING "v0.4.1-beta")
#=============================================================================
function(configure_version NAME_PREFIX)
    cmake_parse_arguments(ARG 
        "" 
        "STRING;FILE" 
        "" 
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
    
    # parse version: Major.Minor.Patch[-BuildMeta]
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

    set(${NAME_PREFIX}_VERSION_FULL     ${RAW_VERSION}  PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_SEMVER   ${SEMVER}       PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_MAJOR    ${MAJOR}        PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_MINOR    ${MINOR}        PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_PATCH    ${PATCH}        PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_BUILD    ${BUILD_META}   PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION          ${SEMVER}       PARENT_SCOPE)
endfunction()