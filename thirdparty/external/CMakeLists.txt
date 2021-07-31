
project(TemplePlusDependencies)
cmake_minimum_required(VERSION 3.15)

set(external_dir ${CMAKE_CURRENT_BINARY_DIR}/externals)
set(installs_dir ${CMAKE_CURRENT_BINARY_DIR}/installs)

set(cmake_args -Wno-dev)
if (CMAKE_CROSSCOMPILING)
    set(cmake_args ${cmake_args} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif ()

include(ExternalProject)
ExternalProject_Add(
        zlib-release
        URL http://zlib.net/zlib-1.2.11.tar.xz
        URL_HASH SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066
        PREFIX ${external_dir}/zlib-release
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=RELWITHDEBINFO" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/zlib-release" ${cmake_args}
)
ExternalProject_Add(
        zlib-debug
        URL http://zlib.net/zlib-1.2.11.tar.xz
        URL_HASH SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066
        PREFIX ${external_dir}/zlib-debug
        CMAKE_ARGS "-DCMAKE_BUILD_TYPE=DEBUG" "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/zlib-debug" ${cmake_args}
)

install(DIRECTORY ${installs_dir}/zlib-release/ DESTINATION zlib)
install(DIRECTORY ${installs_dir}/zlib-debug/bin DESTINATION zlib)
install(DIRECTORY ${installs_dir}/zlib-debug/lib DESTINATION zlib)
# Don't install the headers twice
# install(DIRECTORY ${installs_dir}/zlib-release/include TYPE INCLUDE)
