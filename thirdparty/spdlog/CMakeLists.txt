
file(GLOB_RECURSE SPDLOG_HEADERS "include/*.h")
file(GLOB_RECURSE SPDLOG_SOURCES "src/*.cpp")

add_library(spdlog ${SPDLOG_HEADERS} ${SPDLOG_SOURCES})
target_compile_definitions(spdlog PRIVATE SPDLOG_COMPILED_LIB)
target_compile_definitions(spdlog PUBLIC SPDLOG_FMT_EXTERNAL)
target_include_directories(spdlog PUBLIC include)
target_link_libraries(spdlog PUBLIC fmtlib)
