set(CUBE_MODULE_NAME "Cube")

include("${PROJECT_SOURCE_DIR}/cmake/Modules/SwiftFramework.cmake")

add_swift_framework(cube_framework
    NAME            ${CUBE_MODULE_NAME}
    HEADERS         "${CUBE_BUILD_INCLUDE_DIR}/cube/cube.h"
                    "${CUBE_BUILD_INCLUDE_DIR}/cube/cube_export.h"
                    "${CUBE_BUILD_INCLUDE_DIR}/cube/version.h"
    MODULEMAP       "module.modulemap"
    APINOTES        "Cube.apinotes"
    SOURCES         "Cube.swift"
    OBJECTS         $<TARGET_OBJECTS:${PROJECT_NAME}::objs>
    LINK_FLAGS      "-lc++"
)

add_library("${PROJECT_NAME}::Framework" ALIAS cube_framework)

get_target_property(CUBE_FW_DIR cube_framework FRAMEWORK_PATH)

install(
    DIRECTORY ${CUBE_FW_DIR}
    DESTINATION "Frameworks"
    USE_SOURCE_PERMISSIONS
    COMPONENT SwiftFramework
)