
set(PROJECT_NAME EASTL)

cmake_minimum_required(VERSION 3.15)
project(${PROJECT_NAME} CXX)

find_package(EABase REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

########################################################################
# Extract the version number from EASTL's version.h to avoid having to
# maintain the version in multiple places.
########################################################################
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/EASTL/internal/config.h" EASTL_VERSION_H)
if(EASTL_VERSION_H MATCHES ".*EASTL_VERSION[ \\t]+\"([^\"]+).*$")
    set(PROJECT_VERSION ${CMAKE_MATCH_1})
else()
    message(FATAL_ERROR "Cannot determine EASTL version from config.h: ${EASTL_VERSION_H}")
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

file(GLOB EASTL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_library(EASTL ${EASTL_SOURCES})

# To differentiate between EASTL.lib and EASTLd.lib
set_target_properties(EASTL PROPERTIES VERSION ${PROJECT_VERSION} DEBUG_POSTFIX "d")

target_compile_definitions(EASTL PUBLIC EASTL_OPENSOURCE=1)
# Python 2.7 Headers define ssize_t as long, while EASTL tries to define it as int
# We can only prevent EASTL from defining it again via this define
target_compile_definitions(EASTL PUBLIC _SSIZE_T_DEFINED)
if (MSVC)
    target_compile_definitions(EASTL PUBLIC _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
endif ()

target_link_libraries(EASTL PUBLIC EABase::EABase)

target_include_directories(EASTL PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install(TARGETS EASTL EXPORT ${targets_export_name})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# The following installs are for the CMake package files
install(FILES ${project_config} ${version_config} DESTINATION ${cmake_pkg_install_dir})
install(EXPORT ${targets_export_name} DESTINATION ${cmake_pkg_install_dir} NAMESPACE ${PROJECT_NAME}::)
