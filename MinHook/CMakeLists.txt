
file(GLOB_RECURSE MINHOOK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(minhook STATIC ${MINHOOK_SOURCES})
target_include_directories(
	minhook
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
