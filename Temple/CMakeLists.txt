set(PROJECT_NAME Temple)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "include/temple/dll.h"
    "include/temple/materials.h"
    "include/temple/meshes.h"
    "include/temple/moviesystem.h"
    "include/temple/soundsystem.h"
    "include/temple/vfs.h"
    "include/tig/texture.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/dll.cpp"
    "src/meshes.cpp"
    "src/moviesystem.cpp"
    "src/soundsystem.cpp"
    "src/vfs.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE TempleWrappers)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE     "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_TESTWITHIDA "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebug
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<CONFIG:TestWithIda>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:TestWithIda>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:TestWithIda>:"
        "NDEBUG"
    ">"
    "WIN32;"
    "_CRT_SECURE_NO_WARNINGS;"
    "_SCL_SECURE_NO_WARNINGS;"
    "_LIB;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi
        >
        $<$<CONFIG:TestWithIda>:
            /O2;
            /Oi
        >
        /std:c++17;
        /arch:SSE2;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:TestWithIda>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    Infrastructure
)

# Link with other targets.
target_link_libraries(${PROJECT_NAME} PUBLIC
    Infrastructure
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "minhook_d"
    ">"
    "$<$<CONFIG:Release>:"
        "minhook"
    ">"
    "$<$<CONFIG:TestWithIda>:"
        "minhook"
    ">"
    "Infrastructure"
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PUBLIC
    "${OUTPUT_DIRECTORY}"
)

