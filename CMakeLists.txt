cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(TemplePlus CXX)

# We generally use LTCG a.k.a. INTERPROCEDURAL_OPTIMIZATION
include(CheckIPOSupported)
check_ipo_supported()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

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
    # remove default flags provided with CMake for MSVC
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "")
    set(CMAKE_CXX_FLAGS_TESTWITHIDA "")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
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

add_library(python STATIC IMPORTED GLOBAL)
set_target_properties(python PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/python27.lib)
set_target_properties(python PROPERTIES IMPORTED_LOCATION_DEBUG ${DEPENDENCIES_DIR}lib/python27_d.lib)
target_include_directories(python INTERFACE ${DEPENDENCIES_DIR}/include)

add_library(libeay STATIC IMPORTED GLOBAL)
set_target_properties(libeay PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/libeay32.lib)
target_include_directories(libeay INTERFACE ${DEPENDENCIES_DIR}/include)
add_library(ssleay STATIC IMPORTED GLOBAL)
set_target_properties(ssleay PROPERTIES IMPORTED_LOCATION ${DEPENDENCIES_DIR}lib/ssleay32.lib)
target_include_directories(ssleay INTERFACE ${DEPENDENCIES_DIR}/include)

################################################################################
# Sub-projects
################################################################################
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

