set(PROJECT_NAME Infrastructure)

################################################################################
# Source groups
################################################################################
# set(no_group_source_files
#     "breakpad/LICENSE"
#     "include/spdlog/LICENSE"
# )
# source_group("" FILES ${no_group_source_files})

set(Header_Files
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
    "include/fmt/container.h"
    "include/fmt/format.h"
    "include/fmt/ostream.h"
    "include/fmt/posix.h"
    "include/fmt/printf.h"
    "include/fmt/string.h"
    "include/fmt/time.h"
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
    "include/gsl/span"
    "include/gsl/string_span"
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
    "include/spdlog/async_logger.h"
    "include/spdlog/common.h"
    "include/spdlog/details/async_log_helper.h"
    "include/spdlog/details/async_logger_impl.h"
    "include/spdlog/details/file_helper.h"
    # "include/spdlog/details/line_logger.h"
    "include/spdlog/details/log_msg.h"
    "include/spdlog/details/logger_impl.h"
    "include/spdlog/details/mpmc_blocking_q.h"
    # "include/spdlog/details/mpmc_bounded_q.h"
    "include/spdlog/details/null_mutex.h"
    "include/spdlog/details/os.h"
    "include/spdlog/details/pattern_formatter_impl.h"
    "include/spdlog/details/registry.h"
    "include/spdlog/details/spdlog_impl.h"
    "include/spdlog/fmt/fmt.h"
    "include/spdlog/fmt/ostr.h"
    "include/spdlog/formatter.h"
    "include/spdlog/logger.h"
    "include/spdlog/sinks/android_sink.h"
    "include/spdlog/sinks/ansicolor_sink.h"
    "include/spdlog/sinks/base_sink.h"
    "include/spdlog/sinks/dist_sink.h"
    "include/spdlog/sinks/file_sinks.h"
    "include/spdlog/sinks/msvc_sink.h"
    "include/spdlog/sinks/null_sink.h"
    "include/spdlog/sinks/ostream_sink.h"
    "include/spdlog/sinks/sink.h"
    "include/spdlog/sinks/stdout_sinks.h"
    "include/spdlog/sinks/syslog_sink.h"
    "include/spdlog/sinks/wincolor_sink.h"
    "include/spdlog/sinks/windebug_sink.h"
    "include/spdlog/spdlog.h"
    "include/spdlog/tweakme.h"
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
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "breakpad.cpp"
    "crypto.cpp"
    "d3d.cpp"
    "elfhash.cpp"
    "images.cpp"
    "images_jpeg.cpp"
    "images_tga.cpp"
    "include/fmt/format.cc"
    "include/fmt/ostream.cc"
    "include/fmt/posix.cc"
    "include/fmt/printf.cc"
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
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Infrastructure)

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
message("Infrastructure include directory: ${CMAKE_CURRENT_SOURCE_DIR}/include")
message("Source Dir: ${CMAKE_SOURCE_DIR}")

target_include_directories(${PROJECT_NAME} PUBLIC
    #"/usr/share/mingw-w64/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/dependencies/include"
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
    EASTL
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "$<$<CONFIG:Debug>:"
        "crash_reporting_d"
    ">"
    "$<$<CONFIG:Release>:"
        "crash_reporting"
    ">"
    "$<$<CONFIG:TestWithIda>:"
        "crash_reporting"
    ">"
)
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

