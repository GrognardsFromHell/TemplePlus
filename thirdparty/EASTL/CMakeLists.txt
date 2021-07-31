
file(GLOB EASTL_HEADERS "include/*.h")
file(GLOB EASTL_SOURCES "source/*.cpp")
add_library(EASTL ${EASTL_HEADERS} ${EASTL_SOURCES})

target_compile_definitions(EASTL PUBLIC -D_CHAR16T)
target_compile_definitions(EASTL PUBLIC -D_CRT_SECURE_NO_WARNINGS)
target_compile_definitions(EASTL PUBLIC -D_SCL_SECURE_NO_WARNINGS)
target_compile_definitions(EASTL PUBLIC -DEASTL_OPENSOURCE=1)

target_include_directories(EASTL PUBLIC include)

target_link_libraries(EASTL PUBLIC EABase)
