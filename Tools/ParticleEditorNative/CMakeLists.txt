set(PROJECT_NAME ParticleEditorNative)

################################################################################
# Source groups
################################################################################
set(HEADERS
    "external.h"
)

set(SOURCES
    "animatedmodel.cpp"
    "api.h"
    "dll.cpp"
    "external.cpp"
    "ParticleEditorNative.cpp"
    "video.cpp"
    "video.h"
    "video_avcodec.cpp"
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tools")
target_compile_definitions(${PROJECT_NAME} PRIVATE PARTICLEEDITORNATIVE_EXPORTS)

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC
    Infrastructure
    ParticleSystems
    Temple
    ffmpeg
)
