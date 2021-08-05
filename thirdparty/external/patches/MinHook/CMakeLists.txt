
set(PROJECT_NAME MinHook)

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(${PROJECT_NAME} C)

set(MINHOOK_VERSION "" CACHE STRING "The project version to use. Can't be extracted from source code")
set(PROJECT_VERSION ${MINHOOK_VERSION})

########################################################################
# Creating a reusable CMake package is far more involved than it should be
# We have to:
# 1) Create a "config" file (<project-lowercase>-config.cmake)
# 2) Create a "version" file (<project-lowercase>-version.cmake)
# 3) Export targets (done by the install-commands below)
# 4) Install all three to the installation directory
########################################################################
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(version_config ${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}-config-version.cmake)
set(project_config ${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}-config.cmake)
set(targets_export_name ${PROJECT_NAME_LOWERCASE}-targets)
set(cmake_pkg_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

########################################################################
# Generate a CMake package version file that'll be installed alongside
# the project.
########################################################################
write_basic_package_version_file(${version_config} VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion)

########################################################################
# Generate a CMake package config file that'll be installed alongside
# the project. This is essentially minimal boilerplate.
########################################################################
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in [=[
@PACKAGE_INIT@

include("${CMAKE_CURRENT_LIST_DIR}/@targets_export_name@.cmake")

check_required_components(${PROJECT_NAME})
]=])

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        ${project_config}
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )

########################################################################
# Set up the actual targets
########################################################################

set(SOURCES
    src/hde/hde32.c
    src/buffer.c
    src/hook.c
    src/trampoline.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)

# To differentiate between MinHook.lib and MinHookd.lib
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} DEBUG_POSTFIX "d")
target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Enable PDB files even for static libs
set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_PDB_NAME ${PROJECT_NAME}
        COMPILE_PDB_NAME_DEBUG ${PROJECT_NAME}d
        COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS ${PROJECT_NAME} EXPORT ${targets_export_name})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.pdb DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION include)
install(FILES src/hde/hde32.h src/hde/pstdint.h DESTINATION include/hde)
install(FILES src/trampoline.h DESTINATION include)

# The following installs are for the CMake package files
install(FILES ${project_config} ${version_config} DESTINATION ${cmake_pkg_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${cmake_pkg_install_dir} NAMESPACE ${PROJECT_NAME}::)
