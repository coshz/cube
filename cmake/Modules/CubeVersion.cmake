include_guard(GLOBAL)

#=============================================================================
# configure_version
#=============================================================================
#
# @brief Parses Semantic Versioning (Major.Minor.Patch[-PreRelease+BuildMeta])
#        and exports version variables with a specified prefix.
#
# @param NAME_PREFIX [Required] Positional argument. Variable name prefix
# @param FILE        [Optional] Keyword argument. Path to the version file
# @param STRING      [Optional] Keyword argument. Version string provided directly
#
# @note Exactly one of FILE or STRING must be provided. If both are passed,
#       STRING takes precedence.
#
# Exports (with given prefix `NAME_PREFIX`):
#   _VERSION_MAJOR / _MINOR, _PATCH : Integer literals
#   _VERSION        : Normal version "X.Y.Z"
#   _VERSION_FULL   : Raw input version with leading "v" stripped
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
    
    string(REGEX REPLACE "^[vV]" "" ver_cleaned "${RAW_VERSION}")
    if(NOT ver_cleaned MATCHES "^(([0-9]+)\\.([0-9]+)\\.([0-9]+))(-([A-Za-z0-9.-]+))?(\\+([A-Za-z0-9.-]+))?$")
        message(FATAL_ERROR "Invalid version string: ${RAW_VERSION}")
    endif()

    set(ver_core        "${CMAKE_MATCH_1}")
    set(ver_major       "${CMAKE_MATCH_2}")
    set(ver_minor       "${CMAKE_MATCH_3}")
    set(ver_patch       "${CMAKE_MATCH_4}")
    set(ver_prerelease  "${CMAKE_MATCH_6}")
    set(ver_buildmeta   "${CMAKE_MATCH_8}")
    
    set(${NAME_PREFIX}_VERSION_MAJOR    ${ver_major}    PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_MINOR    ${ver_minor}    PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_PATCH    ${ver_patch}    PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION          ${ver_core}     PARENT_SCOPE)
    set(${NAME_PREFIX}_VERSION_FULL     ${ver_cleaned}  PARENT_SCOPE)
endfunction()