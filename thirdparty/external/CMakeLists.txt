
project(TemplePlusDependencies)
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#############################################################################
# URLs and Content-Hashes for our external dependencies
#############################################################################
set(BREAKPAD_URL https://codeload.github.com/google/breakpad/tar.gz/b324760c7f53667af128a6b77b790323da04fcb9)
set(BREAKPAD_URL_HASH SHA256=abfb52ba17c6387ee2f9a3fcc54673338409772a9a7dbc861ed8334ad62c6bd9)
set(BREAKPAD_VERSION "b324760c")

set(MINHOOK_VERSION 1.3.3)
set(MINHOOK_URL https://codeload.github.com/TsudaKageyu/minhook/tar.gz/refs/tags/v${MINHOOK_VERSION})
set(MINHOOK_URL_HASH SHA256=5bec16358ec9086d4593124bf558635e89135abea2c76e5761ecaf09f4546b19)

set(ZLIB_URL http://zlib.net/zlib-1.2.11.tar.xz)
set(ZLIB_URL_HASH SHA256=4ff941449631ace0d4d203e3483be9dbc9da454084111f97ea0a2114e19bf066)

set(NASM_URL https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win32/nasm-2.15.05-win32.zip)
set(NASM_URL_HASH SHA256=258c7d1076e435511cf2fdf94e2281eadbdb9e3003fd57f356f446e2bce3119e)

set(LIBJPEGTURBO_URL https://codeload.github.com/libjpeg-turbo/libjpeg-turbo/tar.gz/refs/tags/2.1.0)
set(LIBJPEGTURBO_URL_HASH SHA256=d6b7790927d658108dfd3bee2f0c66a2924c51ee7f9dc930f62c452f4a638c52)

set(EABASE_URL https://codeload.github.com/electronicarts/EABase/tar.gz/refs/tags/2.09.05)
set(EABASE_URL_HASH SHA256=f85b98c96f1976aa013c8d6016c5e37a409633ac3fce843cd4ec0b89c67e6b7a)

set(EASTL_URL https://codeload.github.com/electronicarts/EASTL/tar.gz/refs/tags/3.17.06)
set(EASTL_URL_HASH SHA256=9ebeef26cdf091877ee348450d2711cd0bb60ae435309126c0adf8fec9a01ea5)

set(FMTLIB_URL https://github.com/fmtlib/fmt/releases/download/8.0.1/fmt-8.0.1.zip)
set(FMTLIB_URL_HASH SHA256=a627a56eab9554fc1e5dd9a623d0768583b3a383ff70a4312ba68f94c9d415bf)

set(SPDLOG_URL https://codeload.github.com/gabime/spdlog/tar.gz/refs/tags/v1.9.1)
set(SPDLOG_URL_HASH SHA256=9a452cfa24408baccc9b2bc2d421d68172a7630c99e9504a14754be840d31a62)

set(PYBIND11_URL https://codeload.github.com/pybind/pybind11/tar.gz/refs/tags/v2.7.0)
set(PYBIND11_URL_HASH SHA256=6cd73b3d0bf3daf415b5f9b87ca8817cc2e2b64c275d65f9500250f9fee1677e)

set(FFMPEG_DEV_URL https://github.com/GrognardsFromHell/Dependencies/raw/master/ffmpeg-3.4.1-win32-dev.zip)
set(FFMPEG_DEV_URL_HASH SHA256=2cfc802561d68e58ba3185276d244eac3a6163998ab3ea9b8fa457b077a8d7c8)
set(FFMPEG_BIN_URL https://github.com/GrognardsFromHell/Dependencies/raw/master/ffmpeg-3.4.1-win32-shared.zip)
set(FFMPEG_BIN_URL_HASH SHA256=74628044181069d9ea9967a64c196418d1b1d04f724499845a2f36dbf1371d93)

set(DIRECTXMATH_URL https://codeload.github.com/microsoft/DirectXMath/tar.gz/refs/tags/jan2021)
set(DIRECTXMATH_URL_HASH SHA256=d5fff421b15e3815c6a5fcefa257a64780acad0975cd5f45c5428470a39bb9d5)

set(GOOGLETEST_URL https://codeload.github.com/google/googletest/tar.gz/refs/tags/release-1.11.0)
set(GOOGLETEST_URL_HASH SHA256=b4870bf121ff7795ba20d20bcdd8627b8e088f2d1dab299a031c1034eddc93d5)

#############################################################################
# Options to disable individual parts for quicker debugging
#############################################################################
set(BUILD_BREAKPAD ON CACHE BOOL "Include breakpad in build")
set(BUILD_MINHOOK ON CACHE BOOL "Include Minhook in build")
set(BUILD_LIBJPEGTURBO ON CACHE BOOL "Include libjpeg-turbo in build")
set(BUILD_EABASE ON CACHE BOOL "Include EABase in build")
set(BUILD_EAASTL ON CACHE BOOL "Include EASTL in build (implies EABase)")
set(BUILD_FMTLIB ON CACHE BOOL "Include fmtlib in build")
set(BUILD_SPDLOG ON CACHE BOOL "Include spdlog in build (implies fmtlib)")
set(BUILD_ZLIB ON CACHE BOOL "Include zlib in build")
set(BUILD_PYBIND11 ON CACHE BOOL "Include pybind11 in build")
set(BUILD_FFMPEG ON CACHE BOOL "Include ffmpeg in build")
set(BUILD_DIRECTXMATH ON CACHE BOOL "Include DirectXMath in build")
set(BUILD_GOOGLETEST ON CACHE BOOL "Include GoogleTest in build")

#############################################################################
# Use the following for testing individual parts of the build
#############################################################################
#set(BUILD_BREAKPAD OFF)
#set(BUILD_MINHOOK OFF)
#set(BUILD_LIBJPEGTURBO OFF)
#set(BUILD_EABASE OFF)
#set(BUILD_EAASTL OFF)
#set(BUILD_FMTLIB OFF)
#set(BUILD_SPDLOG OFF)
#set(BUILD_ZLIB OFF)
#set(BUILD_PYBIND11 OFF)
#set(BUILD_FFMPEG OFF)
#set(BUILD_DIRECTXMATH OFF)
#set(BUILD_GOOGLETEST OFF)

#############################################################################
# Common Options for building
#############################################################################
set(external_dir ${CMAKE_CURRENT_BINARY_DIR}/externals)
set(installs_dir ${CMAKE_CURRENT_BINARY_DIR}/installs)

set(CMAKE_ARGS -Wno-dev -DCMAKE_CXX_STANDARD=20)
if (CMAKE_CROSSCOMPILING)
    set(CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
endif ()

#############################################################################
# Enable LTCG/IPO if possible
#############################################################################
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)
if (IPO_SUPPORTED)
    set(CMAKE_ARGS ${CMAKE_ARGS} -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON)
else ()
    message("IPO/LTCG disabled because it's unsupported: ${IPO_OUTPUT}")
endif ()

include(ExternalProject)

#############################################################################
# breakpad
#############################################################################
if (BUILD_BREAKPAD)
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                breakpad-${CONFIG}
                URL ${BREAKPAD_URL}
                URL_HASH ${BREAKPAD_URL_HASH}
                PREFIX ${external_dir}/breakpad-${CONFIG}
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DBREAKPAD_VERSION=${BREAKPAD_VERSION}"
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/breakpad-${CONFIG}"
                PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/patches/breakpad" <SOURCE_DIR>
        )
        # This will merge debug/release versions. Destination folder must match CMake package name (breakpad).
        install(DIRECTORY ${installs_dir}/breakpad-${CONFIG}/ DESTINATION breakpad)
    endforeach ()
endif ()

#############################################################################
# Minhook
#############################################################################
if (BUILD_MINHOOK)
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                MinHook-${CONFIG}
                URL ${MINHOOK_URL}
                URL_HASH ${MINHOOK_URL_HASH}
                PREFIX ${external_dir}/MinHook-${CONFIG}
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DMINHOOK_VERSION=${MINHOOK_VERSION}"
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/MinHook-${CONFIG}"
                PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/patches/MinHook" <SOURCE_DIR>
        )
        # This will merge debug/release versions. Destination folder must match CMake package name (MinHook).
        install(DIRECTORY ${installs_dir}/MinHook-${CONFIG}/ DESTINATION MinHook)
    endforeach ()
endif ()

#############################################################################
# libjpeg-turbo
#############################################################################
if (BUILD_LIBJPEGTURBO)
    # Download and extract nasm to build libjpeg-turbo with assembler optimization
    # Note that CMake will extract the zip in the "source" directory named "src",
    # and replace the first level of directory in the downloaded zip with "nasm",
    # so the final executable path is ${external_dir}/nasm/src/nasm/nasm.exe
    ExternalProject_Add(
            nasm
            URL ${NASM_URL}
            URL_HASH ${NASM_URL_HASH}
            PREFIX ${external_dir}/nasm
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
    )

    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                libjpeg-turbo-${CONFIG}
                URL ${LIBJPEGTURBO_URL}
                URL_HASH ${LIBJPEGTURBO_URL_HASH}
                PREFIX ${external_dir}/libjpeg-turbo-${CONFIG}
                DEPENDS nasm
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DCMAKE_ASM_NASM_COMPILER:PATH=${external_dir}/nasm/src/nasm/nasm.exe"
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/libjpeg-turbo-${CONFIG}"
                "-DCMAKE_DEBUG_POSTFIX=d"
                "-DWITH_CRT_DLL=on"
                "-DWITH_JPEG7=no"
                "-DWITH_JPEG8=no"
                "-DENABLE_STATIC=no"
                "-DREQUIRE_SIMD=yes"
        )
        # This will merge debug/release versions. Destination folder must match CMake package name (MinHook).
        install(DIRECTORY ${installs_dir}/libjpeg-turbo-${CONFIG}/ DESTINATION libjpeg-turbo)
    endforeach ()
endif ()

#############################################################################
# EABase (required by EASTL)
# Is header-only
#############################################################################
if (BUILD_EABASE OR BUILD_EAASTL)
    set(EABASE_DIR ${installs_dir}/eabase)
    ExternalProject_Add(
            EABase
            URL ${EABASE_URL}
            URL_HASH ${EABASE_URL_HASH}
            PREFIX ${external_dir}/eabase
            CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${EABASE_DIR}" ${CMAKE_ARGS}
            PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/patches/EABase" <SOURCE_DIR>
    )
    # This will merge debug/release versions. Destination folder must match CMake package name (EABase).
    install(DIRECTORY ${EABASE_DIR}/ DESTINATION EABase)
endif ()

#############################################################################
# EASTL
#############################################################################
if (BUILD_EAASTL)
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                EASTL-${CONFIG}
                URL ${EASTL_URL}
                URL_HASH ${EASTL_URL_HASH}
                PREFIX ${external_dir}/eastl-${CONFIG}
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/eastl-${CONFIG}"
                "-DCMAKE_PREFIX_PATH:PATH=${EABASE_DIR}"
                DEPENDS EABase
                PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/patches/EASTL" <SOURCE_DIR>
        )
        # This will merge debug/release versions. Destination folder must match CMake package name (EASTL).
        install(DIRECTORY ${installs_dir}/eastl-${CONFIG}/ DESTINATION EASTL)
    endforeach ()
endif ()

#############################################################################
# FMTLIB
# https://github.com/fmtlib/fmt
#############################################################################
if (BUILD_FMTLIB OR BUILD_SPDLOG)
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                fmtlib-${CONFIG}
                URL ${FMTLIB_URL}
                URL_HASH ${FMTLIB_URL_HASH}
                PREFIX ${external_dir}/fmtlib-${CONFIG}
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/fmtlib-${CONFIG}"
                "-DFMT_TEST=off"
        )
        # This will merge debug/release versions. Destination folder must match CMake package name (fmt).
        install(DIRECTORY ${installs_dir}/fmtlib-${CONFIG}/ DESTINATION fmt)
    endforeach ()
endif ()

#############################################################################
# SPDLOG
# https://github.com/gabime/spdlog
#############################################################################
if (BUILD_SPDLOG)
    # Make use of our fmtlib instead of bundling it twice
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                spdlog-${CONFIG}
                URL ${SPDLOG_URL}
                URL_HASH ${SPDLOG_URL_HASH}
                DEPENDS fmtlib-${CONFIG}
                PREFIX ${external_dir}/spdlog-${CONFIG}
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_PREFIX_PATH:PATH=${installs_dir}/fmtlib-${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/spdlog-${CONFIG}"
                "-DSPDLOG_FMT_EXTERNAL=ON"
        )
        # This will merge debug/release versions. Destination folder must match CMake package name (spdlog).
        install(DIRECTORY ${installs_dir}/spdlog-${CONFIG}/ DESTINATION spdlog)
    endforeach ()
endif ()

#############################################################################
# ZLIB (Debug + Release)
# https://zlib.net/
#############################################################################
if (BUILD_ZLIB)
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
                zlib-${CONFIG}
                URL ${ZLIB_URL}
                URL_HASH ${ZLIB_URL_HASH}
                PREFIX ${external_dir}/zlib-${CONFIG}
                CMAKE_ARGS ${CMAKE_ARGS}
                "-DCMAKE_BUILD_TYPE=${CONFIG}"
                "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/zlib-${CONFIG}"
                "-DASM686=ON"
                UPDATE_COMMAND ""
                PATCH_COMMAND ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/CMakeLists.txt <SOURCE_DIR>/CMakeLists.org.txt
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/patches/zlib" <SOURCE_DIR>
        )
        # This will merge debug/release versions
        install(DIRECTORY ${installs_dir}/zlib-${CONFIG}/ DESTINATION zlib)
    endforeach ()
endif ()

#############################################################################
# ffmpeg (prebuilt win32 binaries)
#
#############################################################################
if (BUILD_FFMPEG)
    ExternalProject_Add(
            ffmpeg-bin
            URL ${FFMPEG_BIN_URL}
            URL_HASH ${FFMPEG_BIN_URL_HASH}
            PREFIX ${external_dir}/ffmpeg-bin
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${installs_dir}/ffmpeg
    )
    ExternalProject_Add(
            ffmpeg-dev
            URL ${FFMPEG_DEV_URL}
            URL_HASH ${FFMPEG_DEV_URL_HASH}
            PREFIX ${external_dir}/ffmpeg-dev
            DEPENDS ffmpeg-bin
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${installs_dir}/ffmpeg
    )
    install(DIRECTORY ${installs_dir}/ffmpeg/ DESTINATION ffmpeg)
endif ()

#############################################################################
# pybind11
# https://github.com/pybind/pybind11/
# Is header-only
#############################################################################
if (BUILD_PYBIND11)
    ExternalProject_Add(
            pybind11
            URL ${PYBIND11_URL}
            URL_HASH ${PYBIND11_URL_HASH}
            PREFIX ${external_dir}/pybind11
            CMAKE_ARGS ${CMAKE_ARGS}
            "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/pybind11"
            "-DPYBIND11_TEST=OFF"
            "-DPYBIND11_NOPYTHON=ON"
    )
    install(DIRECTORY ${installs_dir}/pybind11/ DESTINATION pybind11)
endif ()

#############################################################################
# DirectXMath
# https://github.com/microsoft/DirectXMath
# Is header-only
#############################################################################
if (BUILD_DIRECTXMATH)
    ExternalProject_Add(
            DirectXMath
            URL ${DIRECTXMATH_URL}
            URL_HASH ${DIRECTXMATH_URL_HASH}
            PREFIX ${external_dir}/DirectXMath
            CMAKE_ARGS ${CMAKE_ARGS}
            "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/DirectXMath"
    )
    install(DIRECTORY ${installs_dir}/DirectXMath/ DESTINATION DirectXMath)
endif ()

#############################################################################
# GoogleTest
# https://github.com/google/googletest/
#############################################################################
if (BUILD_GOOGLETEST)
    foreach (CONFIG IN ITEMS relwithdebinfo debug)
        ExternalProject_Add(
            GTest-${CONFIG}
            URL ${GOOGLETEST_URL}
            URL_HASH ${GOOGLETEST_URL_HASH}
            PREFIX ${external_dir}/GTest-${CONFIG}
            CMAKE_ARGS ${CMAKE_ARGS}
            "-DBUILD_SHARED_LIBS=ON"
            "-DCMAKE_BUILD_TYPE=${CONFIG}"
            "-DCMAKE_INSTALL_PREFIX:PATH=${installs_dir}/GTest-${CONFIG}"
        )
        # This will merge debug/release versions
        install(DIRECTORY ${installs_dir}/GTest-${CONFIG}/ DESTINATION GTest)
    endforeach ()
endif ()
