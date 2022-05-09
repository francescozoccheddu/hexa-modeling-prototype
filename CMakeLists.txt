cmake_minimum_required (VERSION 3.8)

project ("hexa-modeling-prototype")

# cinolib
set (CINOLIB_HEADER_ONLY OFF)
set (CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
set (cinolib_DIR "${PROJECT_SOURCE_DIR}/external/cinolib")
find_package (cinolib REQUIRED)

# ogdf
add_subdirectory ("external/ogdf")

# cpputils
add_subdirectory ("external/cpputils")

# hexa-modeling-prototype
add_subdirectory ("core")
add_subdirectory ("gui")