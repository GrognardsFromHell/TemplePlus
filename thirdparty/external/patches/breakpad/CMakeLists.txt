
set(PROJECT_NAME breakpad)

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(${PROJECT_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BREAKPAD_VERSION "" CACHE STRING "The version of breakpad being built. Can't be discovered from the source.")
set(PROJECT_VERSION ${BREAKPAD_VERSION})

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

set(BREAKPAD_SOURCES
        src/client/windows/handler/exception_handler.cc
        src/client/windows/crash_generation/crash_generation_client.cc
        src/common/windows/guid_string.cc
        )

set(SOURCES crash_reporting.cpp crash_reporting.h)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${BREAKPAD_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
# The following enables symbol export
target_compile_definitions(${PROJECT_NAME} PRIVATE BREAKPAD_EXPORTS)

# To differentiate between breakpad.lib and breakpadd.lib
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} DEBUG_POSTFIX "d")
target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS ${PROJECT_NAME} EXPORT ${targets_export_name})
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION bin OPTIONAL)
install(FILES crash_reporting.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/breakpad)

# The following installs are for the CMake package files
install(FILES ${project_config} ${version_config} DESTINATION ${cmake_pkg_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${cmake_pkg_install_dir} NAMESPACE ${PROJECT_NAME}::)
