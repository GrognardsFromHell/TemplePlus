
add_library(EABase INTERFACE)
target_include_directories(EABase INTERFACE include/Common)
target_compile_definitions(EABase INTERFACE D_CHAR16T)
