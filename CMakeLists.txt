cmake_minimum_required (VERSION 3.8)

project ("hexa-modeling-prototype")

# Setup cinolib
set (CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
set (cinolib_DIR "${CMAKE_CURRENT_LIST_DIR}/external/cinolib")
find_package (cinolib REQUIRED)

# Add subprojects
add_subdirectory ("core")
add_subdirectory ("gui")