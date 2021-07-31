set(PROJECT_NAME ParticleSystems)

################################################################################
# Source groups
################################################################################
set(HEADERS
    "include/particles/alignedarray.h"
    "include/particles/bones.h"
    "include/particles/external.h"
    "include/particles/instances.h"
    "include/particles/params.h"
    "include/particles/parser.h"
    "include/particles/parser_keyframes.h"
    "include/particles/parser_params.h"
    "include/particles/render.h"
    "include/particles/simulation.h"
    "include/particles/spec.h"
    "include/particles/types.h"
    "src/render_general.h"
)

set(SOURCES
    "src/bones.cpp"
    "src/instances.cpp"
    "src/params.cpp"
    "src/parser.cpp"
    "src/parser_keyframes.cpp"
    "src/parser_params.cpp"
    "src/render.cpp"
    "src/render_general.cpp"
    "src/render_model.cpp"
    "src/render_quad.cpp"
    "src/simulation.cpp"
    "src/spec.cpp"
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC include)

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC Infrastructure)
