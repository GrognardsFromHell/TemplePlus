
file(GLOB_RECURSE SOURCES_CPP "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE SOURCES_CC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_library(infrastructure STATIC ${SOURCES_CPP} ${SOURCES_CC} ${HEADERS})

target_include_directories(
	infrastructure
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(infrastructure eastl turbojpeg dwrite.lib dxgi.lib d3d11.lib d2d1.lib d3dcompiler.lib dxguid.lib)
