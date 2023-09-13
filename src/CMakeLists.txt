#----------------------------------------------------------------------------------------------------------------------
# Target setttings
#----------------------------------------------------------------------------------------------------------------------

add_executable(${PROJECT_NAME} main.cpp ${CMAKE_SOURCE_DIR}/resources/windows.rc)
target_link_libraries(${PROJECT_NAME} nvapiwrapper)
