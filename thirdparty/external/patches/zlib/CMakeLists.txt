
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# We handle target installs later
set(SKIP_INSTALL_LIBRARIES ON)
include(CMakeLists.org.txt)

# Inherit some props from the original zlib CMakeLists.txt
set(PROJECT_VERSION ${VERSION})

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

target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS zlib EXPORT ${targets_export_name})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>.pdb DESTINATION ${CMAKE_INSTALL_LIBDIR})

# The following installs are for the CMake package files
install(FILES ${project_config} ${version_config} DESTINATION ${cmake_pkg_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${cmake_pkg_install_dir} NAMESPACE ${PROJECT_NAME}::)
