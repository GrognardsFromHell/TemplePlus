
add_library(gmock
        "gtest/src/gtest-all.cc"
        src/gmock-all.cc)

target_include_directories(gmock PRIVATE . gtest)
target_include_directories(gmock PUBLIC include gtest/include)
message("${DEFAULT_CXX_EXCEPTION_HANDLING} ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}")

add_library(gmock_main
        "gtest/src/gtest-all.cc"
        src/gmock-all.cc
        src/gmock_main.cc)

target_include_directories(gmock_main PRIVATE . gtest)
target_include_directories(gmock_main PUBLIC include gtest/include)
