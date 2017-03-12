
file(GLOB_RECURSE PARTSYS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(partsys STATIC ${PARTSYS_SOURCES})
target_include_directories(
	partsys
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(partsys infrastructure)
