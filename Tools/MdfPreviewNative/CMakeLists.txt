set(PROJECT_NAME MdfPreviewNative)

################################################################################
# Source groups
################################################################################
set(HEADERS
    "api.h"
)
set(SOURCES
    "native.cpp"
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tools")

target_compile_definitions(${PROJECT_NAME} PRIVATE MDFPREVIEWNATIVE_EXPORTS)

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC
    Infrastructure
    ParticleSystems
    Temple
)
