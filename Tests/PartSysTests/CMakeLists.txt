set(PROJECT_NAME PartSysTests)

set(HEADERS
    "stdafx.h"
    "targetver.h"
)
set(SOURCES
    "alignedarray_test.cpp"
    "main.cpp"
    "mesparser_test.cpp"
    "parser_param_test.cpp"
    "parser_test.cpp"
    "partsyskeyframes_test.cpp"
    "partsysrandom_test.cpp"
    "simulation_test.cpp"
    "stdafx.cpp"
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")

add_precompiled_header(${PROJECT_NAME} "stdafx.h" "stdafx.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE
    Infrastructure
    ParticleSystems
    gmock_main
)
