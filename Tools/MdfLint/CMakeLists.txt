set(PROJECT_NAME MdfLint)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} MdfLint.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tools")
target_link_libraries(${PROJECT_NAME} PRIVATE Infrastructure)
