cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
cmake_policy(SET CMP0111 NEW)

# Generally target Windows 10 SDK
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(TemplePlus CXX)

include(CMake/DefaultBuildType.cmake)

# We generally use LTCG a.k.a. INTERPROCEDURAL_OPTIMIZATION, but do not require it
include(CheckIPOSupported)
check_ipo_supported(RESULT ltcg_result OUTPUT ltcg_output)
if(ltcg_result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION true)
else()
    message(WARNING "IPO/LTCG is not supported: ${ltcg_output}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Copy everything to the same output directory to avoid trouble with DLLs not being found
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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
    # /RTC1 -> Runtime error checks

    # Flags for all configs
    set(COMMON_CXX_FLAGS "/EHsc /Zi /arch:SSE2 /W3")
    message("${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_CXX_FLAGS} /Od /RTC1")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_CXX_FLAGS} /O2 /Oi /DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

    add_compile_definitions(
            _CRT_SECURE_NO_WARNINGS # Disables nagging about fopen_s etc.
            _SCL_SECURE_NO_WARNINGS
            UNICODE # needed to get the Unicode versions of Win32 APIs
            _UNICODE # needed to get the Unicode versions of MSVCRT APIs
            WIN32 # Generally set on Windows
    )
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

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

