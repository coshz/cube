# SwiftFramework.cmake — provides add_swift_framework(), which assembles a
# versioned macOS .framework bundle:
#
#   <Name>.framework/
#       <Name>            -> Versions/Current/<Name>
#       Headers           -> Versions/Current/Headers
#       Modules           -> Versions/Current/Modules
#       Versions/
#           A/
#               <Name>                 (the compiled binary)
#               Headers/<header>, <Name>.apinotes
#               Modules/module.modulemap, <Name>.swiftmodule/...
#           Current -> A
#
# ...acting as a Swift overlay of a same-named Clang module, and compiles
# Swift sources into it. This file knows nothing about any specific
# project — every detail is a named argument.
#
# Usage:
#   include(cmake/SwiftFramework.cmake)
#   add_swift_framework(toy_framework
#       NAME          Toy (optional; defaults to <TARGET_NAME>)
#       SOURCES       swift/Toy.swift
#       HEADERS       include/toy.h
#       MODULEMAP     swift/module.modulemap
#       APINOTES      swift/Toy.apinotes (optional)               
#       OBJECTS       $<TARGET_OBJECTS:toy_objects> (optional)
#       LINK_FLAGS    "-lc++" (optional)          
#   )
#
# Defines target <TARGET_NAME> (added to ALL). If swiftc isn't available
# or the platform isn't Apple, emits a warning and defines an empty target
# instead of failing the configure — callers don't need their own guard.

find_program(SWIFTC_EXECUTABLE swiftc)
find_program(LIPO_EXECUTABLE lipo)

function(add_swift_framework TARGET_NAME)
    if(TARGET "${TARGET_NAME}")
        message(FATAL_ERROR "add_swift_framework(): Target '${TARGET_NAME}' already exists!")
    elseif(NOT APPLE OR NOT SWIFTC_EXECUTABLE OR NOT LIPO_EXECUTABLE)
        message(WARNING "add_swift_framework(): Non-Apple platform or swiftc/lipo missing -- skipping. ")
        add_library("${TARGET_NAME}" INTERFACE)
        return()
    endif()

    cmake_parse_arguments(ARG
        ""
        "NAME;MODULEMAP;APINOTES"
        "HEADERS;SOURCES;OBJECTS;LINK_FLAGS"
        ${ARGN}
    )

    if(NOT ARG_MODULEMAP)
        message(FATAL_ERROR "add_swift_framework(): MODULEMAP is required!")
    endif()
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "add_swift_framework(): SOURCES is required!")
    endif()
    if(NOT ARG_NAME)
        set(ARG_NAME "${TARGET_NAME}")
    endif()

    _make_absolute(ARG_MODULEMAP)
    _make_absolute(ARG_APINOTES)
    _make_absolute(ARG_HEADERS)
    _make_absolute(ARG_SOURCES)
    
    set(fw_target       "${TARGET_NAME}")
    set(fw_name         "${ARG_NAME}.framework")
    set(fw_dir          "${CMAKE_CURRENT_BINARY_DIR}/${fw_name}")
    set(fw_ver          "A")
    set(ver_dir         "${fw_dir}/Versions/${fw_ver}")
    set(ver_headers     "${ver_dir}/Headers")
    set(ver_modules     "${ver_dir}/Modules")
    set(ver_swiftmodule "${ver_modules}/${ARG_NAME}.swiftmodule")
    set(ver_binary      "${ver_dir}/${ARG_NAME}")
    
    set(sdk_path        ${CMAKE_OSX_SYSROOT})
    if(NOT sdk_path)
        execute_process(
            COMMAND xcrun --show-sdk-path 
            OUTPUT_VARIABLE sdk_path 
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    # ------------------------------------------------------------------
    # 1) prepare — Versions/<v>/{Headers,Modules} + the Current/ Symlinks
    #   Must exist before compiling: that's how `-import-underlying-module` 
    #   finds the Clang half of the module.
    # ------------------------------------------------------------------
    set(prepare_stamp "${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME}_framework_prepare.stamp")

    set(prepare_steps
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ver_headers}" "${ver_modules}"
        COMMAND ${CMAKE_COMMAND} -E copy "${ARG_MODULEMAP}" "${ver_modules}/module.modulemap"
    )

    foreach(header IN LISTS ARG_HEADERS)
        get_filename_component(header_name "${header}" NAME)
        list(APPEND prepare_steps
            COMMAND ${CMAKE_COMMAND} -E copy "${header}" "${ver_headers}/${header_name}"
        )
    endforeach()

    if(ARG_APINOTES)
        list(APPEND prepare_steps
            COMMAND ${CMAKE_COMMAND} -E copy "${ARG_APINOTES}" "${ver_headers}/${ARG_NAME}.apinotes"
        )
    endif()

    string(TOLOWER "${ARG_NAME}" HEADER_PREFIX )
    list(APPEND prepare_steps
        COMMAND ${CMAKE_COMMAND} -E create_symlink "." "${ver_headers}/${HEADER_PREFIX}"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${fw_ver}" "${fw_dir}/Versions/Current"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/Headers" "${fw_dir}/Headers"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/Modules" "${fw_dir}/Modules"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "Versions/Current/${ARG_NAME}" "${fw_dir}/${ARG_NAME}"
        COMMAND ${CMAKE_COMMAND} -E touch "${prepare_stamp}"
    )

    add_custom_command(
        OUTPUT  "${prepare_stamp}"
        ${prepare_steps}
        DEPENDS ${ARG_HEADERS} ${ARG_MODULEMAP} ${ARG_APINOTES}
        COMMENT "Preparing ${fw_name} layout"
        VERBATIM
    )

    # ------------------------------------------------------------------
    # 2) compile — multi-architecture compilation && binaries merging
    # ------------------------------------------------------------------
    set(target_archs ${CMAKE_OSX_ARCHITECTURES})
    if(NOT target_archs)
        set(target_archs "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    
    set(slice_binaries "")
    set(swiftc_commands "")

    foreach(arch IN LISTS target_archs)
        set(min_ver "11.0")
        if(arch STREQUAL "x86_64")
            set(min_ver "10.15")
        endif()
        set(effective_ver ${CMAKE_OSX_DEPLOYMENT_TARGET})
        if(NOT effective_ver OR effective_ver VERSION_LESS min_ver)
            set(effective_ver ${min_ver})
        endif()

        set(triple "${arch}-apple-macosx${effective_ver}")
        set(slice_bin "${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME}_${triple}.slice")
        list(APPEND slice_binaries "${slice_bin}")
        list(APPEND swiftc_commands
            COMMAND ${SWIFTC_EXECUTABLE}
                    -emit-library
                    -emit-module 
                    -enable-library-evolution
                    -target ${triple}
                    -emit-module-path "${ver_swiftmodule}/${arch}-apple-macos.swiftmodule"
                    -emit-module-interface-path "${ver_swiftmodule}/${arch}-apple-macos.swiftinterface"
                    -import-underlying-module -module-name "${ARG_NAME}"
                    -sdk "${sdk_path}"
                    -F "${CMAKE_CURRENT_BINARY_DIR}"
                    -Xlinker -install_name -Xlinker "@rpath/${fw_name}/${ARG_NAME}"
                    ${ARG_LINK_FLAGS}
                    -o "${slice_bin}"
                    ${ARG_SOURCES} ${ARG_OBJECTS}
        )
    endforeach()

    add_custom_command(
        OUTPUT  ${ver_binary}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ver_swiftmodule}"
        ${swiftc_commands}
        COMMAND ${LIPO_EXECUTABLE} -create ${slice_binaries} -output "${ver_binary}"
        DEPENDS "${prepare_stamp}" ${ARG_SOURCES} ${ARG_OBJECTS}
        COMMAND_EXPAND_LISTS
        COMMENT "Compiling ${fw_name} [${target_archs}]"
        VERBATIM
    )

    # ------------------------------------------------------------------
    # 3) Target export 
    # ------------------------------------------------------------------

    set(fw_build_target "${ARG_NAME}_framework_build")
    add_custom_target("${fw_build_target}" ALL DEPENDS "${ver_binary}")

    add_library(${fw_target} INTERFACE)
    add_dependencies(${fw_target} "${fw_build_target}")

    target_compile_options(${fw_target} INTERFACE 
        $<BUILD_INTERFACE:-F${CMAKE_CURRENT_BINARY_DIR}>
    )
    target_link_libraries(${fw_target} INTERFACE 
        $<BUILD_INTERFACE:-F${CMAKE_CURRENT_BINARY_DIR}>
        "-framework ${ARG_NAME}"
    )
    set_target_properties(${fw_target} PROPERTIES
        FRAMEWORK_NAME "${ARG_NAME}"
        FRAMEWORK_PATH "${fw_dir}"
    )
endfunction()


macro(_make_absolute paths)
    if(DEFINED ${paths} AND NOT "${${paths}}" STREQUAL "")
        set(abs_paths "")
        foreach(path IN LISTS ${paths})
            if(NOT IS_ABSOLUTE "${path}")
                list(APPEND abs_paths "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
            else()
                list(APPEND abs_paths "${path}")
            endif()
        endforeach()
        set(${paths} ${abs_paths})
    endif()
endmacro()