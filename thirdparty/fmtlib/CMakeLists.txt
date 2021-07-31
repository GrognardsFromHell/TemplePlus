
file(GLOB_RECURSE FMTLIB_HEADERS "include/*.h")
file(GLOB_RECURSE FMTLIB_SOURCES src/format.cc src/os.cc)

add_library(fmtlib ${FMTLIB_HEADERS} ${FMTLIB_SOURCES})
target_include_directories(fmtlib PUBLIC include)
