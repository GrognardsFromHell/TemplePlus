set(PROJECT_NAME Temple)

################################################################################
# Source groups
################################################################################
set(HEADERS
    "include/temple/dll.h"
    "include/temple/materials.h"
    "include/temple/meshes.h"
    "include/temple/moviesystem.h"
    "include/temple/soundsystem.h"
    "include/temple/vfs.h"
    "include/tig/texture.h"
)

set(SOURCES
    "src/dll.cpp"
    "src/meshes.cpp"
    "src/moviesystem.cpp"
    "src/soundsystem.cpp"
    "src/vfs.cpp"
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC include)

################################################################################
# Dependencies
################################################################################

target_link_libraries(${PROJECT_NAME} PRIVATE Infrastructure minhook)

# Needed for EnumProcessModules/GetModuleInformation
target_link_libraries(${PROJECT_NAME} PUBLIC psapi.lib)
