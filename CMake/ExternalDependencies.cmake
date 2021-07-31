
######################################################################
# The following is inspired by FreeMat
######################################################################

set(external_base_dir ${CMAKE_CURRENT_BINARY_DIR}/external)
set(external_install_dir ${external_base_dir}/install)

set(THIRDPARTY_EXTERNAL_PACKAGES_DIR ${CMAKE_SOURCE_DIR}/thirdparty/external/packages)

######################################################################
# ZLIB Package
######################################################################

if (EXISTS ${THIRDPARTY_EXTERNAL_PACKAGES_DIR}/zlib AND NOT DEFINED ZLIB_ROOT)
    set(ZLIB_ROOT ${THIRDPARTY_EXTERNAL_PACKAGES_DIR}/zlib)
    message("Attempting to use zlib from ${ZLIB_ROOT}")
endif ()

find_package(ZLIB REQUIRED)

######################################################################
# Python2 Package
######################################################################

find_package(Python2 REQUIRED COMPONENTS Interpreter Development)
