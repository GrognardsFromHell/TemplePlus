
set(DEPENDENCY_DIR ${CMAKE_SOURCE_DIR}/dependencies)

if (NOT IS_DIRECTORY "${DEPENDENCY_DIR}") 
	message(FATAL_ERROR "Unable to find dependencies in ${DEPENDENCY_DIR}")
endif()

message("Using dependencies from ${DEPENDENCY_DIR}")

#
# TurboJPEG
#
add_library(turbojpeg STATIC IMPORTED)
set_target_properties(turbojpeg PROPERTIES 
	IMPORTED_LOCATION_DEBUG "${DEPENDENCY_DIR}/lib/turbojpeg-static_d.lib"
	IMPORTED_LOCATION_RELEASE "${DEPENDENCY_DIR}/lib/turbojpeg-static.lib"
	INTERFACE_INCLUDE_DIRECTORIES "${DEPENDENCY_DIR}/include"
)

#
# OpenSSL
#
add_library(ssleay STATIC IMPORTED)
set_target_properties(ssleay PROPERTIES
	IMPORTED_LOCATION "${DEPENDENCY_DIR}/lib/ssleay32.lib"
)
add_library(libeay STATIC IMPORTED)
set_target_properties(libeay PROPERTIES
	IMPORTED_LOCATION "${DEPENDENCY_DIR}/lib/libeay32.lib"
)

#
# Python
#
add_library(python STATIC IMPORTED)
set_target_properties(python PROPERTIES
	IMPORTED_LOCATION_DEBUG "${DEPENDENCY_DIR}/lib/python27_d.lib"
	IMPORTED_LOCATION_RELEASE "${DEPENDENCY_DIR}/lib/python27.lib"
	INTERFACE_INCLUDE_DIRECTORIES "${DEPENDENCY_DIR}/include"
	INTERFACE_LINK_LIBRARIES "ssleay;libeay;crypt32;Ws2_32"
	INTERFACE_COMPILE_DEFINITIONS "Py_NO_ENABLE_SHARED;Py_TRACE_REFS"
)
