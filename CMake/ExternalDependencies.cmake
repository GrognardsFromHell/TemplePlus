
######################################################################
# External Python Install
# This needs to come before pybind11 so it won't search again
######################################################################

find_package(Python2 REQUIRED COMPONENTS Interpreter Development)

######################################################################
# We use 3rd-party dependencies from a directory in our source-tree
# Developers can either download pre-built dependencies, or
# run regenerate.cmd in the dependencies directory to build
# dependencies fitting their compiler version.
######################################################################

set(THIRDPARTY_PACKAGES_DIR ${CMAKE_SOURCE_DIR}/thirdparty/external/packages)

if (NOT EXISTS ${THIRDPARTY_PACKAGES_DIR})
    message(FATAL_ERROR "The third party dependency directory is missing: ${THIRDPARTY_PACKAGES_DIR}.
        You need to either download a prebuilt dependency package or run regenerate.cmd")
endif ()

######################################################################
# Our own pre-built dependencies
######################################################################

find_package(breakpad CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(MinHook CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(EABase CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(EASTL CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(fmt CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(spdlog CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(libjpeg-turbo CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(pybind11 CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})
find_package(zlib CONFIG REQUIRED PATHS ${THIRDPARTY_PACKAGES_DIR})

######################################################################
# Dump Version Info
######################################################################

message("Python Version: ${Python2_VERSION}")
message("breakpad Version: ${breakpad_VERSION}")
message("MinHook Version: ${MinHook_VERSION}")
message("EABase Version: ${EABase_VERSION}")
message("EASTL Version: ${EASTL_VERSION}")
message("fmt Version: ${fmt_VERSION}")
message("spdlog Version: ${spdlog_VERSION}")
message("pybind11 Version: ${pybind11_VERSION}")
message("zlib Version: ${zlib_VERSION}")
message("libjpeg-turbo Version: ${libjpeg-turbo_VERSION}")

######################################################################
# ffmpeg
######################################################################
add_library(ffmpeg::swscale UNKNOWN IMPORTED)
set_target_properties(ffmpeg::swscale PROPERTIES IMPORTED_LOCATION ${THIRDPARTY_PACKAGES_DIR}/ffmpeg/lib/swscale.lib)
add_library(ffmpeg::avcodec UNKNOWN IMPORTED)
set_target_properties(ffmpeg::avcodec PROPERTIES IMPORTED_LOCATION ${THIRDPARTY_PACKAGES_DIR}/ffmpeg/lib/avcodec.lib)
add_library(ffmpeg::avformat UNKNOWN IMPORTED)
set_target_properties(ffmpeg::avformat PROPERTIES IMPORTED_LOCATION ${THIRDPARTY_PACKAGES_DIR}/ffmpeg/lib/avformat.lib)
add_library(ffmpeg::avutil UNKNOWN IMPORTED)
set_target_properties(ffmpeg::avutil PROPERTIES IMPORTED_LOCATION ${THIRDPARTY_PACKAGES_DIR}/ffmpeg/lib/avutil.lib)

add_library(ffmpeg INTERFACE IMPORTED)
target_include_directories(ffmpeg INTERFACE ${THIRDPARTY_PACKAGES_DIR}/ffmpeg/include)
target_link_libraries(ffmpeg INTERFACE ffmpeg::swscale ffmpeg::avcodec ffmpeg::avformat ffmpeg::avutil)

set(IMPORTED_TARGETS
        breakpad::breakpad
        libjpeg-turbo::turbojpeg
        zlib::zlib
)

######################################################################
# Copy all binary dependencies to their output directories
######################################################################
get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
function(copy_dependencies CONFIG)
    set(OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG}}")
    if (NOT "${OUTPUT_DIR}")
        set(OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()

    foreach (TARGET IN LISTS IMPORTED_TARGETS)
        get_target_property(LOCATION ${TARGET} LOCATION_${CONFIG})
        if (NOT "${LOCATION}")
            get_target_property(LOCATION ${TARGET} LOCATION)
        endif()
        file(INSTALL ${LOCATION} DESTINATION ${OUTPUT_DIR})
    endforeach ()
endfunction()
if (${GENERATOR_IS_MULTI_CONFIG})
    foreach (CONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
        message("Copying dependency DLLs for config ${CONFIG}")
        copy_dependencies(${CONFIG})
    endforeach ()
else ()
    message("Copying dependency DLLs")
    copy_dependencies(${CMAKE_BUILD_TYPE})
endif ()
