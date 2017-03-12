
cmake_minimum_required(VERSION 3.6)

project(templeplus)

# output include dirs (good for debugging)
# set(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)

# Enable multi processor compilation
set(CMAKE_CXX_FLAGS "/MP /EHsc")
set(CMAKE_C_FLAGS "/MP")

# Disable security settings because ToEE itself is already a swiss cheese in that regard
set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")

include(dependencies.cmake)

# Put output into a bin directory so that the DLLs can find each other
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Prefer Unicode over MBCS
add_definitions(-DUNICODE -D_UNICODE)

#
# Pick up QT
#
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED HINTS $ENV{QTDIR})
find_package(Qt5Gui REQUIRED HINTS $ENV{QTDIR})
find_package(Qt5QML REQUIRED HINTS $ENV{QTDIR})
find_package(Qt5Quick REQUIRED HINTS $ENV{QTDIR})
find_package(Qt5Network REQUIRED HINTS $ENV{QTDIR})

# Copy over Qt5 into output directory
# Get the directory where the Qt library resides so we can add it to the execution path below
get_target_property(QT_LOCATION Qt5::Core LOCATION)
get_filename_component(QT_DIR ${QT_LOCATION} DIRECTORY)

set(QT5_RELEASE_DLLS ${QT_DIR}/Qt5Core.dll ${QT_DIR}/Qt5Gui.dll ${QT_DIR}/Qt5Qml.dll ${QT_DIR}/Qt5Quick.dll ${QT_DIR}/Qt5Network.dll)
file(COPY ${QT5_RELEASE_DLLS} DESTINATION "${CMAKE_BINARY_DIR}/bin/Release")

set(QT5_DEBUG_DLLS ${QT_DIR}/Qt5Cored.dll ${QT_DIR}/Qt5Guid.dll ${QT_DIR}/Qt5Qmld.dll ${QT_DIR}/Qt5Quickd.dll ${QT_DIR}/Qt5Networkd.dll)
file(COPY ${QT5_DEBUG_DLLS} DESTINATION "${CMAKE_BINARY_DIR}/bin/Debug")

add_subdirectory(MinHook)
add_subdirectory(EASTL)
add_subdirectory(Infrastructure)
add_subdirectory(ParticleSystems)
add_subdirectory(Temple)
add_subdirectory(TemplePlus)
add_subdirectory(TemplePlusLauncher)
