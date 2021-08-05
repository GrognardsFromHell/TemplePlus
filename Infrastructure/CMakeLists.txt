set(PROJECT_NAME Infrastructure)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC EASTL::EASTL fmt::fmt spdlog::spdlog Microsoft::DirectXMath)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC include)

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PRIVATE breakpad::breakpad libjpeg-turbo::turbojpeg)

# System library dependencies for Direct3D/DirectWrite/Direct2D
target_link_libraries(${PROJECT_NAME} PUBLIC
        d2d1.lib
        dwrite.lib
        dxguid.lib
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
)
