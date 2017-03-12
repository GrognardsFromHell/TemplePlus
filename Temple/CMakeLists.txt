
file(GLOB_RECURSE TEMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(temple STATIC ${TEMPLE_SOURCES})
target_include_directories(
	temple
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(temple minhook infrastructure)
