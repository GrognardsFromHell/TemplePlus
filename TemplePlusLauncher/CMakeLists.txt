
add_executable(templepluslauncher main.cpp resource.h resource.rc)

# Clear the /RTC compiler flag for this target since it requires the CRT
# Courtesy of StackOverflow (http://stackoverflow.com/questions/8587764/remove-runtime-checks-compiler-flag-per-project-in-cmake)
foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    STRING (REGEX REPLACE "/RTC[^ ]*" "" ${flag_var} "${${flag_var}}")
endforeach(flag_var)

# Get the directory where the Qt library resides so we can add it to the execution path below
get_target_property(QT_LOCATION Qt5::Core LOCATION)
get_filename_component(QT_DIR ${QT_LOCATION} DIRECTORY)

target_link_libraries(templepluslauncher templeplus)
set_target_properties(templepluslauncher PROPERTIES
	 LINK_FLAGS "/NODEFAULTLIB /SUBSYSTEM:WINDOWS"
	 # we have to disable certain security checks for this EXE that rely on the CRT (which we disabled)
	 COMPILE_FLAGS "/GS-"
	 ENVIRONMENT "PATH=\$(PATH);$QT_DIR"
	 PDB_NAME TemplePlusLauncher # Avoid conflict with TemplePlus.dll PDB
	 OUTPUT_NAME "TemplePlus")
