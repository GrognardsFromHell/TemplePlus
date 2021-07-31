
file(GLOB EASTL_HEADERS "include/*.h")
file(GLOB EASTL_SOURCES "source/*.cpp")
add_library(EASTL ${EASTL_HEADERS} ${EASTL_SOURCES})

target_compile_definitions(EASTL PUBLIC
        # The Python 2.7 headers ssize_t as long, while EASTL uses int
        # The definitions are binary-compatible, but MSVC complains
        -D_SSIZE_T_DEFINED
        -D_CHAR16T
)
target_compile_definitions(EASTL PUBLIC -DEASTL_OPENSOURCE=1)

target_include_directories(EASTL PUBLIC include)

target_link_libraries(EASTL PUBLIC EABase)
