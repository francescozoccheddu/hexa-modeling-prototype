cmake_minimum_required (VERSION 3.8)

project ("hexa-modeling-prototype")

# Set C++ standard
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Setup cinolib
set (CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
set (CINOLIB_USES_TETGEN ON)
set (CINOLIB_USES_TRIANGLE ON)
set (CINOLIB_USES_EXACT_PREDICATES ON)
set (CINOLIB_USES_GRAPH_CUT OFF)
set (CINOLIB_USES_BOOST ON)
set (CINOLIB_USES_VTK OFF)
set (cinolib_DIR "${CMAKE_CURRENT_LIST_DIR}/external/cinolib")
add_compile_definitions (CINOLIB_DIR="${cinolib_DIR}") # TODO Remove
find_package (cinolib REQUIRED)

# Add subprojects
add_subdirectory ("gui")