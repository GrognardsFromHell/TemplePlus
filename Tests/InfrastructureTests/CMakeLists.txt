set(PROJECT_NAME InfrastructureTests)

################################################################################
# Source groups
################################################################################
set(HEADERS
    "stdafx.h"
    "targetver.h"
)
set(SOURCES
    "main.cpp"
    "stdafx.cpp"
    "tokenizer_test.cpp"
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")
add_precompiled_header(${PROJECT_NAME} "stdafx.h" "stdafx.cpp")

################################################################################
# Dependencies
################################################################################

target_link_libraries(${PROJECT_NAME} PRIVATE
    Infrastructure
    gmock_main
)
