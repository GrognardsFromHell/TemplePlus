set(PROJECT_NAME Infrastructure)

################################################################################
# Source groups
################################################################################

set(HEADERS
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
target_link_libraries(${PROJECT_NAME} PUBLIC EASTL fmt::fmt spdlog)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC include)

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PRIVATE crash_reporting libjpegturbo)

# System library dependencies for Direct3D/DirectWrite/Direct2D
target_link_libraries(${PROJECT_NAME} PUBLIC
        d2d1.lib
        dwrite.lib
        dxguid.lib
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
)
