set(NAME_MODULE DavaTools)

set(MODULE_TYPE STATIC)

set(HPP_FILES_RECURSE Sources/Tools/*.h)
set(CPP_FILES_RECURSE Sources/Tools/*.cpp)

set(DavaToolsRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

set(INCLUDES "${DavaToolsRootDir}/Sources")
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set(DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set(DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

set(FIND_PACKAGE NetworkHelpers)

setup_main_module()
