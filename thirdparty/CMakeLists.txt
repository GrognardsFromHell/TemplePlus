
#add_subdirectory(EABase)
#add_subdirectory(EASTL)
add_subdirectory(googlemock)

# We require CMake 3.15, but some of our subprojects don't
# We want to use the newer behavior for LTCG/IPO
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

include(FetchContent)
