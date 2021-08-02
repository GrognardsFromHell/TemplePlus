
set(PROJECT_NAME EABase)

cmake_minimum_required(VERSION 3.15)
project(${PROJECT_NAME} CXX)

########################################################################
# Extract the version number from EABase's version.h to avoid having to
# maintain the version in multiple places.
########################################################################
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/Common/EABase/version.h" EABASE_VERSION_H)
if(EABASE_VERSION_H MATCHES ".*EABASE_VERSION[ \\t]+\"([^\"]+).*$")
    set(PROJECT_VERSION ${CMAKE_MATCH_1})
else()
    message(FATAL_ERROR "Cannot determine EABase version from version.h: ${EABASE_VERSION_H}")
endif()

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
add_library(EABase INTERFACE)
target_compile_definitions(EABase INTERFACE _CHAR16T)

# The generator expression is used to differentiate between the include-path when
# this target is in-tree and when it is imported from an installed CMake package
target_include_directories(EABase INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/Common>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

########################################################################
# Configure the installation
########################################################################
install(TARGETS EABase EXPORT ${targets_export_name})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/Common/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# The following installs are for the CMake package files
install(FILES ${project_config} ${version_config} DESTINATION ${cmake_pkg_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${cmake_pkg_install_dir} NAMESPACE EABase::)
