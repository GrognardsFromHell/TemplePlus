
file(GLOB_RECURSE EASTL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

add_library(eastl STATIC ${EASTL_SOURCES})
target_include_directories(
	eastl
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
