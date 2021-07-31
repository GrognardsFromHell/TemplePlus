set(PROJECT_NAME Infrastructure)

################################################################################
# Source groups
################################################################################

set(HEADERS
    # "breakpad/breakpad_types.h"
    # "breakpad/crash_generation_client.h"
    # "breakpad/exception_handler.h"
    # "breakpad/guid_string.h"
    # "breakpad/ipc_protocol.h"
    # "breakpad/minidump_cpu_amd64.h"
    # "breakpad/minidump_cpu_x86.h"
    # "breakpad/minidump_exception_win32.h"
    # "breakpad/minidump_format.h"
    # "breakpad/scoped_ptr.h"
    # "breakpad/string_utils-inl.h"
    "include/aas/aas.h"
    "include/aas/aas_math.h"
    "include/aas/aas_model_factory.h"
    "include/aas/aas_renderer.h"
    "include/debugui.h"
    "include/graphics/bufferbinding.h"
    "include/graphics/buffers.h"
    "include/graphics/camera.h"
    "include/graphics/collision.h"
    "include/graphics/device.h"
    "include/graphics/dynamictexture.h"
    "include/graphics/materials.h"
    "include/graphics/math.h"
    "include/graphics/mdfmaterials.h"
    "include/graphics/shaders.h"
    "include/graphics/shaperenderer2d.h"
    "include/graphics/shaperenderer3d.h"
    "include/graphics/textengine.h"
    "include/graphics/textures.h"
    "include/gsl/gsl"
    "include/gsl/gsl_assert"
    "include/gsl/gsl_util"
    "include/imconfig.h"
    "include/imgui.h"
    "include/infrastructure/binaryreader.h"
    "include/infrastructure/breakpad.h"
    "include/infrastructure/crypto.h"
    "include/infrastructure/elfhash.h"
    "include/infrastructure/exception.h"
    "include/infrastructure/images.h"
    "include/infrastructure/infrastructure.h"
    "include/infrastructure/INI.h"
    "include/infrastructure/json11.hpp"
    "include/infrastructure/keyboard.h"
    "include/infrastructure/location.h"
    "include/infrastructure/logging.h"
    "include/infrastructure/macros.h"
    "include/infrastructure/mathutil.h"
    "include/infrastructure/mdfmaterial.h"
    "include/infrastructure/mdfparser.h"
    "include/infrastructure/meshes.h"
    "include/infrastructure/mesparser.h"
    "include/infrastructure/stopwatch.h"
    "include/infrastructure/stringutil.h"
    "include/infrastructure/tabparser.h"
    "include/infrastructure/tokenizer.h"
    "include/infrastructure/version.h"
    "include/infrastructure/vfs.h"
    "include/platform/d3d.h"
    "include/platform/windows.h"
    "src/aas/aas_anim_events.h"
    "src/aas/aas_anim_player.h"
    "src/aas/aas_anim_player_stream.h"
    "src/aas/aas_animated_model.h"
    "src/aas/aas_cloth_collision.h"
    "src/aas/aas_cloth_sim.h"
    "src/aas/aas_cloth_sim_mesh.h"
    "src/aas/aas_mesh.h"
    "src/aas/aas_skeleton.h"
    "src/debugui/imgui/imgui_impl_dx11.h"
    "src/graphics/shaders_compiler.h"
    "stb_image.h"
)

set(SOURCES
    "breakpad.cpp"
    "crypto.cpp"
    "d3d.cpp"
    "elfhash.cpp"
    "images.cpp"
    "images_jpeg.cpp"
    "images_tga.cpp"
    "json11.cpp"
    "keyboard.cpp"
    "logging.cpp"
    "mdfparser.cpp"
    "meshes_animfallbacks.cpp"
    "mesparser.cpp"
    "src/aas/aas.cpp"
    "src/aas/aas_anim_player.cpp"
    "src/aas/aas_anim_player_stream.cpp"
    "src/aas/aas_animated_model.cpp"
    "src/aas/aas_cloth_collision.cpp"
    "src/aas/aas_cloth_sim.cpp"
    "src/aas/aas_cloth_sim_mesh.cpp"
    "src/aas/aas_math.cpp"
    "src/aas/aas_mesh.cpp"
    "src/aas/aas_model_factory.cpp"
    "src/aas/aas_renderer.cpp"
    "src/aas/aas_skeleton.cpp"
    "src/allocator.cpp"
    "src/debugui/debugui.cpp"
    "src/debugui/imgui/imgui.cpp"
    "src/debugui/imgui/imgui_demo.cpp"
    "src/debugui/imgui/imgui_draw.cpp"
    "src/debugui/imgui/imgui_impl_dx11.cpp"
    "src/graphics/bufferbinding.cpp"
    "src/graphics/buffers.cpp"
    "src/graphics/camera.cpp"
    "src/graphics/device.cpp"
    "src/graphics/dynamictexture.cpp"
    "src/graphics/mdfmaterials.cpp"
    "src/graphics/shaders.cpp"
    "src/graphics/shaders_compiler.cpp"
    "src/graphics/shaperenderer2d.cpp"
    "src/graphics/shaperenderer3d.cpp"
    "src/graphics/textengine.cpp"
    "src/graphics/textures.cpp"
    "src/location.cpp"
    "src/meshes.cpp"
    "stringutil.cpp"
    "tabparser.cpp"
    "tokenizer.cpp"
    "version.cpp"
    "vfs.cpp"
    "windows.cpp"
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC EASTL fmtlib spdlog)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

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
    #"/usr/share/mingw-w64/include"
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
    "_LIB;"
    "$ENV{MSBuildDefines};"
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
        /arch:SSE2;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
endif()

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PRIVATE EASTL crash_reporting)

