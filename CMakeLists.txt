cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Generally target Windows 10 SDK
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(TemplePlus CXX)

# We generally use LTCG a.k.a. INTERPROCEDURAL_OPTIMIZATION, but do not rely on it
include(CheckIPOSupported)
check_ipo_supported(RESULT ltcg_result OUTPUT ltcg_output)
if(ltcg_result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
else()
    message(WARNING "IPO/LTCG is not supported: ${ltcg_output}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

################################################################################
# Configuration properties set by the CI/CD for making release builds
################################################################################
set(TP_VERSION "" CACHE STRING "Sets the Temple+ Version Number")
set(TP_COMMIT "" CACHE STRING "Sets the Git Commit that Temple+ is being built from")
set(TP_RELEASE_BUILD OFF CACHE BOOL "Indicate that a release is being built that is for use outside of the dev environment")

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "Win32")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "TestWithIda"
    CACHE STRING "" FORCE
)

################################################################################
# Global compiler options
################################################################################
if(MSVC)
    #
    # Compile Options:
    # /EHsc -> Exceptions enabled
    # /Zi -> debug info in separate PDB file
    # /Od -> turns off all optimizations
    # /O2 -> maximize speed
    # /Oi -> enable intrinsic functions
    # /arch:SSE2 -> allow unconditional use of SSE2
    # /W3 -> warnings, warnings, warnings

    # Flags for all configs
    set(COMMON_CXX_FLAGS "/EHsc /Zi /arch:SSE2 /W3")
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_CXX_FLAGS} /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_CXX_FLAGS} /O2 /Oi")
    set(CMAKE_CXX_FLAGS_TESTWITHIDA "${CMAKE_CXX_FLAGS_RELEASE}")

    add_compile_definitions(
            _CRT_SECURE_NO_WARNINGS # Disables nagging about fopen_s etc.
            _SCL_SECURE_NO_WARNINGS
            UNICODE # needed to get the Unicode versions of Win32 APIs
            _UNICODE # needed to get the Unicode versions of MSVCRT APIs
            WIN32 # Generally set on Windows
            # The following sets NDEBUG / _DEBUG accordingly
            "$<$<CONFIG:Debug>:_DEBUG>"
            "$<$<CONFIG:Release>:NDEBUG>"
            "$<$<CONFIG:TestWithIda>:NDEBUG>"
    )
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    #
    # Linker Options:
    # /DEBUG disables /OPT:ICF and /OPT:REF
    #

    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_TESTWITHIDA "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_TESTWITHIDA "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_TESTWITHIDA "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_TESTWITHIDA "${CMAKE_STATIC_LINKER_FLAGS}")
endif()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##### Define pseudo-target for dependencies
set(DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/)

add_library(crash_reporting STATIC IMPORTED GLOBAL)
set_target_properties(crash_reporting PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/crash_reporting.lib)
set_target_properties(crash_reporting PROPERTIES IMPORTED_LOCATION_DEBUG ${DEPENDENCIES_DIR}lib/crash_reporting_d.lib)
target_include_directories(crash_reporting INTERFACE ${DEPENDENCIES_DIR}/include)

add_library(minhook STATIC IMPORTED GLOBAL)
set_target_properties(minhook PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/minhook.lib)
set_target_properties(minhook PROPERTIES IMPORTED_LOCATION_DEBUG ${DEPENDENCIES_DIR}lib/minhook_d.lib)
target_include_directories(minhook INTERFACE ${DEPENDENCIES_DIR}/include)

add_library(libjpegturbo STATIC IMPORTED GLOBAL)
set_target_properties(libjpegturbo PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/turbojpeg-static.lib)
set_target_properties(libjpegturbo PROPERTIES IMPORTED_LOCATION_DEBUG ${DEPENDENCIES_DIR}lib/turbojpeg-static_d.lib)
target_include_directories(libjpegturbo INTERFACE ${DEPENDENCIES_DIR}/include)

#add_library(python STATIC IMPORTED GLOBAL)
#set_target_properties(python PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/python27.lib)
#set_target_properties(python PROPERTIES IMPORTED_LOCATION_DEBUG ${DEPENDENCIES_DIR}lib/python27_d.lib)
#target_include_directories(python INTERFACE ${DEPENDENCIES_DIR}/include)

#add_library(libeay STATIC IMPORTED GLOBAL)
#set_target_properties(libeay PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/libeay32.lib)
#target_include_directories(libeay INTERFACE ${DEPENDENCIES_DIR}/include)
#add_library(ssleay STATIC IMPORTED GLOBAL)
#set_target_properties(ssleay PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/ssleay32.lib)
#target_include_directories(ssleay INTERFACE ${DEPENDENCIES_DIR}/include)

################################################################################
# FFMPEG Dependency
################################################################################
add_library(ffmpeg::swscale UNKNOWN IMPORTED)
set_target_properties(ffmpeg::swscale PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/swscale.lib)
add_library(ffmpeg::avcodec UNKNOWN IMPORTED)
set_target_properties(ffmpeg::avcodec PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/avcodec.lib)
add_library(ffmpeg::avformat UNKNOWN IMPORTED)
set_target_properties(ffmpeg::avformat PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/avformat.lib)
add_library(ffmpeg::avutil UNKNOWN IMPORTED)
set_target_properties(ffmpeg::avutil PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/avutil.lib)

add_library(ffmpeg INTERFACE IMPORTED)
target_include_directories(ffmpeg INTERFACE ${DEPENDENCIES_DIR}/include)
target_link_libraries(ffmpeg INTERFACE ffmpeg::swscale ffmpeg::avcodec ffmpeg::avformat ffmpeg::avutil)

################################################################################
# Sub-projects
################################################################################
include(CMake/ExternalDependencies.cmake)
add_subdirectory(thirdparty)
add_subdirectory(Infrastructure)
add_subdirectory(ParticleSystems)
add_subdirectory(Temple)
add_subdirectory(TemplePlus)
add_subdirectory(Tests/InfrastructureTests)
add_subdirectory(Tests/PartSysTests)
add_subdirectory(Tools/MdfLint)
add_subdirectory(Tools/MdfPreviewNative)
add_subdirectory(Tools/ParticleEditorNative)

