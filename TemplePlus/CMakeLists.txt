
file(GLOB_RECURSE TP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE TP_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(templeplus SHARED ${TP_SOURCES} ${TP_HEADERS})

#
# Find and expose full path to tpdata directory
#
get_filename_component(TPDATA ${CMAKE_CURRENT_SOURCE_DIR}/../tpdata ABSOLUTE)
add_definitions("-DTPDATAPATH=${TPDATA}")

add_definitions(-DQT_STATICPLUGIN -DPYBIND11_EXPORT)

target_include_directories(
	templeplus
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(templeplus PRIVATE infrastructure temple partsys eastl python Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick ${CMAKE_CURRENT_SOURCE_DIR}/tio/tio.lib psapi.lib)
include_directories(${Qt5Quick_PRIVATE_INCLUDE_DIRS})

# tio.dll must be delay loaded since it's inaccessible before we change the DLL search path after the program has
# already been started
set_target_properties(templeplus PROPERTIES 
	LINK_FLAGS /DELAYLOAD:tio.dll
	AUTOMOC_MOC_OPTIONS "-Muri=TemplePlus"
)
