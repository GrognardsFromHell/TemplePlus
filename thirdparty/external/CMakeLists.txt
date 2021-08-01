
project(TemplePlusDependencies)
cmake_minimum_required(VERSION 3.15)

#############################################################################
# URLs and Content-Hashes for our external dependencies
#############################################################################
set(ZLIB_URL http://zlib.net/zlib-1.2.11.tar.xz)
set(ZLIB_URL_HASH SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066)

set(EABASE_URL https://codeload.github.com/electronicarts/EABase/tar.gz/refs/tags/2.09.05)
set(EABASE_URL_HASH SHA256=f85b98c96f1976aa013c8d6016c5e37a409633ac3fce843cd4ec0b89c67e6b7a)

set(EASTL_URL https://codeload.github.com/electronicarts/EASTL/tar.gz/refs/tags/3.17.06)
set(EASTL_URL_HASH SHA256=9ebeef26cdf091877ee348450d2711cd0bb60ae435309126c0adf8fec9a01ea5)

set(FMTLIB_URL https://github.com/fmtlib/fmt/releases/download/8.0.1/fmt-8.0.1.zip)
set(FMTLIB_URL_HASH SHA256=a627a56eab9554fc1e5dd9a623d0768583b3a383ff70a4312ba68f94c9d415bf)

set(SPDLOG_URL https://codeload.github.com/gabime/spdlog/tar.gz/refs/tags/v1.9.1)
set(SPDLOG_URL_HASH SHA256=9a452cfa24408baccc9b2bc2d421d68172a7630c99e9504a14754be840d31a62)

set(BUILD_EABASE ON CACHE BOOL "Include EABase in build")
set(BUILD_EAASTL ON CACHE BOOL "Include EASTL in build (implies EABase)")
set(BUILD_FMTLIB ON CACHE BOOL "Include fmtlib in build")
set(BUILD_SPDLOG ON CACHE BOOL "Include spdlog in build (implies fmtlib)")
set(BUILD_ZLIB ON CACHE BOOL "Include zlib in build")

#############################################################################
# Use the following for testing individual parts of the build
#############################################################################
set(BUILD_EABASE OFF)
#set(BUILD_EAASTL OFF)
set(BUILD_FMTLIB OFF)
set(BUILD_SPDLOG OFF)
set(BUILD_ZLIB OFF)

#############################################################################
# Common Options for building
#############################################################################
set(external_dir ${CMAKE_CURRENT_BINARY_DIR}/externals)
set(installs_dir ${CMAKE_CURRENT_BINARY_DIR}/installs)

set(cmake_args -Wno-dev)
if (CMAKE_CROSSCOMPILING)
    set(cmake_args ${cmake_args} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif ()

include(ExternalProject)

#############################################################################
# EABase (required by EASTL)
# Is header-only
#############################################################################
if(BUILD_EABASE OR BUILD_EAASTL)
set(EABASE_DIR ${installs_dir}/eabase)
ExternalProject_Add(
        EABase
        URL ${EABASE_URL}
        URL_HASH ${EABASE_URL_HASH}
        PREFIX ${external_dir}/eabase
        CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${EABASE_DIR}" ${cmake_args}
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EABase.cmake" <SOURCE_DIR>/CMakeLists.txt
)
endif()

#############################################################################
# EASTL
# Is header-only
#############################################################################
if(BUILD_EAASTL)
set(eastl_cmake_args "-DCMAKE_PREFIX_PATH:PATH=${EABASE_DIR}" ${cmake_args})
ExternalProject_Add(
        EASTL-debug
        URL ${EASTL_URL}
        URL_HASH ${EASTL_URL_HASH}
        PREFIX ${external_dir}/eastl-debug
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=DEBUG" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/eastl-debug" ${eastl_cmake_args}
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EASTL.cmake" <SOURCE_DIR>/CMakeLists.txt
)
add_dependencies(EASTL-debug EABase)
ExternalProject_Add(
        EASTL-release
        URL ${EASTL_URL}
        URL_HASH ${EASTL_URL_HASH}
        PREFIX ${external_dir}/eastl-release
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=RELWITHDEBINFO" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/eastl-release" ${eastl_cmake_args}
        PATCH_COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EASTL.cmake" <SOURCE_DIR>/CMakeLists.txt
)
add_dependencies(EASTL-release EABase)
endif()

# Python 2.7 Headers define ssize_t as long, while EASTL tries to define it as int
# We can only prevent EASTL from defining it again via this define
#target_compile_definitions(EASTL PUBLIC _SSIZE_T_DEFINED)

#############################################################################
# FMTLIB
# https://github.com/fmtlib/fmt
#############################################################################
if(BUILD_FMTLIB)
ExternalProject_Add(
        fmtlib-release
        URL ${FMTLIB_URL}
        URL_HASH ${FMTLIB_URL_HASH}
        PREFIX ${external_dir}/fmtlib-release
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=RELWITHDEBINFO" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/fmtlib-release" ${cmake_args}
)
ExternalProject_Add(
        fmtlib-debug
        URL ${FMTLIB_URL}
        URL_HASH ${FMTLIB_URL_HASH}
        PREFIX ${external_dir}/fmtlib-debug
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=DEBUG" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/fmtlib-debug" ${cmake_args}
)

# This will overwrite the include directory which exists in both
install(DIRECTORY ${installs_dir}/fmtlib-debug/ DESTINATION fmtlib)
install(DIRECTORY ${installs_dir}/fmtlib-release/ DESTINATION fmtlib)
endif()

#############################################################################
# SPDLOG
# https://github.com/gabime/spdlog
#############################################################################
if(BUILD_SPDLOG)
# Make use of our fmtlib instead of bundling it twice
set(SPDLOG_FMT_EXTERNAL ON)
ExternalProject_Add(
        spdlog-release
        URL ${SPDLOG_URL}
        URL_HASH ${SPDLOG_URL_HASH}
        PREFIX ${external_dir}/spdlog-release
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=RELWITHDEBINFO" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/spdlog-release" ${cmake_args}
)
ExternalProject_Add(
        spdlog-debug
        URL ${SPDLOG_URL}
        URL_HASH ${SPDLOG_URL_HASH}
        PREFIX ${external_dir}/spdlog-debug
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=DEBUG" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/spdlog-debug" ${cmake_args}
)
endif()

#############################################################################
# ZLIB (Debug + Release)
# https://zlib.net/
#############################################################################
if(BUILD_ZLIB)
ExternalProject_Add(
        zlib-release
        URL ${ZLIB_URL}
        URL_HASH ${ZLIB_URL_HASH}
        PREFIX ${external_dir}/zlib-release
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=RELWITHDEBINFO" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/zlib-release" ${cmake_args}
)
ExternalProject_Add(
        zlib-debug
        URL ${ZLIB_URL}
        URL_HASH ${ZLIB_URL_HASH}
        PREFIX ${external_dir}/zlib-debug
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=DEBUG" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/zlib-debug" ${cmake_args}
)

# This will overwrite the include directory which exists in both
install(DIRECTORY ${installs_dir}/zlib-debug/ DESTINATION zlib)
install(DIRECTORY ${installs_dir}/zlib-release/ DESTINATION zlib)
endif()
